#include <fs/utility/assert.hpp>
#include <fs/utility/visitor.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/detail/autogen.hpp>
#include <fs/lang/conditions.hpp>
#include <fs/lang/item.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/constants.hpp>
#include <fs/lang/market/item_price_data.hpp>
#include <fs/lang/loot/knowledge.hpp>

#include <cmath>
#include <limits>
#include <string_view>
#include <initializer_list>
#include <type_traits>
#include <variant>

namespace fs::compiler::detail {

namespace {

// ---- item search/match functions (basic blocks for a single item) ----

template <typename Container, typename Predicate>
[[nodiscard]] lang::condition_values_container<lang::string>
get_matching_items(
	const Container& container,
	Predicate predicate,
	lang::price_range_condition price_range,
	lang::position_tag autogen_origin)
{
	using container_value_type = typename Container::value_type;
	static_assert(
		std::is_base_of_v<lang::market::elementary_item, container_value_type>,
		"items container should store elementary_item or derived type");
	static_assert(
		std::is_invocable_r_v<bool, Predicate, container_value_type>,
		"Predicate should accept Container's element and return bool");

	lang::condition_values_container<lang::string> result;
	for (const auto& item : container) {
		if (item.price.confidence == lang::market::confidence_level::low || !price_range.includes(item.price.chaos_value))
			continue;

		if (!predicate(item))
			continue;

		result.push_back(lang::string{item.name, autogen_origin});
	}

	return result;
}

struct empty_matcher
{
	bool operator()(const lang::market::elementary_item& /* itm */)
	{
		return true;
	}
};

struct poe1_gem_matcher
{
	int level;
	int quality;
	bool corrupt;

	bool operator()(const lang::market::poe1::gem& g) const
	{
		return g.level == level && g.quality == quality && g.is_corrupted == corrupt;
	}
};

// ---- item search/match functions (blocks with StackSize condition) ----

// An item eligible for a block with a partcicular StackSize condition
struct eligible_item
{
	std::string_view name;
	std::optional<int> amount_min;
	std::optional<int> amount_max;

	bool has_same_amounts_as(eligible_item other) const
	{
		return amount_min == other.amount_min && amount_max == other.amount_max;
	}
};

// clangd may underline this function as unused but it is used by std::sort in this file
bool operator<(eligible_item lhs, eligible_item rhs)
{
	const int lhs_min = lhs.amount_min.value_or(-1);
	const int rhs_min = rhs.amount_min.value_or(-1);
	if (lhs_min != rhs_min)
		return lhs_min < rhs_min;

	const int lhs_max = lhs.amount_max.value_or(std::numeric_limits<int>::max());
	const int rhs_max = rhs.amount_max.value_or(std::numeric_limits<int>::max());
	if (lhs_max != rhs_max)
		return lhs_max < rhs_max;

	return false;
}

// From market_items, get a list of items that match specified price_range.
// Each item will have minimum and/or maximum stack size computed, based on known_items.
// Returned items will be sorted by min and max amount fields for efficient block generation.
[[nodiscard]] std::vector<eligible_item> get_eligible_items(
	lang::price_range_condition price_range,
	const std::vector<lang::market::elementary_item>& market_items,
	const lang::loot::known_items_store& known_items,
	lang::generated_blocks_consumer consumer)
{
	std::vector<eligible_item> eligible_items;
	// there will be at most market_items.size() eligible items
	eligible_items.reserve(market_items.size());

	for (const lang::market::elementary_item& market_item : market_items) {
		eligible_item itm;

		std::optional<int> max_stack_size = known_items.max_stack_size_of(market_item.name);
		if (!max_stack_size) {
			consumer.report_unknown_item(market_item.name);
			max_stack_size = lang::autogen_extension::unknown_item_assumed_max_stack_size;
		}

		if (price_range.lower_bound()) {
			const auto amount_min = static_cast<int>(std::ceil(
				(*price_range.lower_bound()).bound.value.value / market_item.price.chaos_value));

			if (amount_min > *max_stack_size) {
				// item too cheap, requires higher amount than the stack allows
				continue;
			}

			itm.amount_min = amount_min;
		}

		if (price_range.upper_bound()) {
			const auto amount_max = static_cast<int>(std::floor(
				(*price_range.upper_bound()).bound.value.value / market_item.price.chaos_value));

			// skip items that can not fit between mix and max price bound
			if (itm.amount_min.value_or(0) > amount_max)
				continue;

			if (amount_max == 0) {
				// item too expensive, even stack of 1 would exceed upper bound
				continue;
			}

			itm.amount_max = std::min(amount_max, *max_stack_size);
		}

		// since at least 1 price bound must be specified, at least 1 amount bound should be computed
		FS_ASSERT(itm.amount_min.has_value() || itm.amount_max.has_value());

		itm.name = market_item.name;
		eligible_items.push_back(itm);
	}

	std::sort(eligible_items.begin(), eligible_items.end());
	return eligible_items;
}

// ---- block generation functions ----

template <typename MarketItemType>
void generate_blocks_simple(
	const lang::block_generation_info& block_info,
	const std::vector<MarketItemType>& item_price_data_field,
	std::string_view item_class_name,
	lang::generated_blocks_consumer consumer)
{
	static_assert(std::is_base_of_v<lang::market::elementary_item, MarketItemType>);

	lang::item_filter_block block(block_info.visibility);
	block.actions = block_info.actions;
	block.continuation = block_info.continuation;

	// Class == @item_class_name
	block.conditions.conditions.push_back(lang::make_class_condition(
		lang::equality_comparison_type::exact_match,
		lang::condition_values_container<lang::string>{
			lang::string{std::string(item_class_name), block_info.autogen_origin}
		},
		block_info.autogen_origin));

	// BaseType == @get_mathing_items()
	block.conditions.conditions.push_back(lang::make_base_type_condition(
		lang::equality_comparison_type::exact_match,
		get_matching_items(item_price_data_field, empty_matcher{}, block_info.price_range, block_info.autogen_origin),
		block_info.autogen_origin));

	if (block.conditions.is_valid())
		consumer.push(std::move(block));
}

template <typename MarketItemType>
void generate_blocks_stackable_item(
	const lang::block_generation_info& block_info,
	const std::vector<MarketItemType>& item_price_data_field,
	std::string_view item_class_name,
	const lang::loot::known_items_store& known_items,
	lang::generated_blocks_consumer consumer)
{
	static_assert(std::is_base_of_v<lang::market::elementary_item, MarketItemType>);

	const std::vector<eligible_item> eligible_items = get_eligible_items(
		block_info.price_range, item_price_data_field, known_items, consumer);
	FS_ASSERT(std::is_sorted(eligible_items.begin(), eligible_items.end()));

	for (auto it = eligible_items.begin(); it != eligible_items.end();) {
		// Find a range of items with same amount requirements (eligible items are sorted by it).
		// Then for each group of items with same requirements, generate a block.
		// Grouping by StackSize avoids excessive generation of a block for each item.
		const eligible_item& first_item = *it;
		const auto last_item_it = std::find_if_not(it, eligible_items.end(), [&first_item](eligible_item ei) {
			return ei.has_same_amounts_as(first_item);
		});

		lang::item_filter_block block(block_info.visibility);
		block.actions = block_info.actions;
		block.continuation = block_info.continuation;

		// Class == @item_class_name
		block.conditions.conditions.push_back(lang::make_class_condition(
			lang::equality_comparison_type::exact_match,
			lang::condition_values_container<lang::string>{
				lang::string{std::string(item_class_name), block_info.autogen_origin}
			},
			block_info.autogen_origin));

		// BaseType == @base_types
		lang::string_comparison_condition::container_type base_types;
		for (; it != last_item_it; ++it) {
			base_types.push_back(lang::string{std::string(it->name), block_info.autogen_origin});
		}
		block.conditions.conditions.push_back(lang::make_base_type_condition(
			lang::equality_comparison_type::exact_match,
			std::move(base_types),
			block_info.autogen_origin));

		// StackSize >= @amount_min
		if (first_item.amount_min) {
			block.conditions.conditions.push_back(lang::make_stack_size_range_bound_condition(
				lang::range_bound<lang::integer>{lang::integer{*first_item.amount_min, block_info.autogen_origin}, true},
				true,
				block_info.autogen_origin
			));
		}

		// StackSize <= @amount_max
		if (first_item.amount_max) {
			block.conditions.conditions.push_back(lang::make_stack_size_range_bound_condition(
				lang::range_bound<lang::integer>{lang::integer{*first_item.amount_max, block_info.autogen_origin}, true},
				false,
				block_info.autogen_origin
			));
		}

		// iterating through eligible_items should never produce blocks with empty BaseType
		FS_ASSERT(block.conditions.is_valid());
		consumer.push(std::move(block));
	}
}

void poe1_generate_blocks_gems(
	const lang::block_generation_info& block_info,
	const lang::market::poe1::item_price_data& item_price_data,
	lang::generated_blocks_consumer consumer)
{
	for (int level = lang::constants::min_item_gem_level; level <= lang::constants::max_item_gem_level; ++level) {
		for (int quality = lang::constants::min_item_gem_quality; quality <= lang::constants::max_item_gem_quality; ++quality) {
			for (bool corrupt : {false, true}) {
				lang::item_filter_block block(block_info.visibility);
				block.actions = block_info.actions;
				block.continuation = block_info.continuation;

				block.conditions.conditions.push_back(lang::make_class_condition(
					lang::equality_comparison_type::exact_match,
					lang::condition_values_container<lang::string>{
						lang::string{lang::item_class_names::gems_active, block_info.autogen_origin},
						lang::string{lang::item_class_names::gems_support, block_info.autogen_origin}
					},
					block_info.autogen_origin));
				block.conditions.conditions.push_back(lang::make_base_type_condition(
					lang::equality_comparison_type::exact_match,
					get_matching_items(
						item_price_data.gems,
						poe1_gem_matcher{level, quality, corrupt},
						block_info.price_range,
						block_info.autogen_origin
					),
					block_info.autogen_origin));
				block.conditions.conditions.push_back(lang::make_gem_level_value_list_condition(
					lang::condition_values_container<lang::integer>{lang::integer{level, block_info.autogen_origin}},
					true,
					block_info.autogen_origin));
				block.conditions.conditions.push_back(lang::make_quality_value_list_condition(
					lang::condition_values_container<lang::integer>{lang::integer{quality, block_info.autogen_origin}},
					true,
					block_info.autogen_origin));
				block.conditions.conditions.push_back(lang::make_corrupted_condition(
					lang::boolean{corrupt, block_info.autogen_origin},
					block_info.autogen_origin));

				if (block.conditions.is_valid())
					consumer.push(std::move(block));
			}
		}
	}
}

// ---- verification functions ----

template <typename F>
[[nodiscard]] bool
verify_autogen_conditions(
	settings st,
	const lang::official_conditions& conditions,
	F condition_verifier,
	lang::position_tag autogen_origin,
	diagnostics_store& diagnostics)
{
	static_assert(std::is_invocable_r_v<bool, F, const lang::official_condition&, lang::position_tag, diagnostics_store&>);

	bool result = true;

	for (const auto& cond : conditions.conditions) {
		if (!condition_verifier(*cond, autogen_origin, diagnostics)) {
			if (st.error_handling.stop_on_error)
				return false;
			else
				result = false;
		}
	}

	return result;
}

struct class_condition_verifier
{
	std::string_view item_class_name;

	[[nodiscard]] bool operator()(
		const lang::official_condition& condition,
		lang::position_tag autogen_origin,
		diagnostics_store& diagnostics) const
	{
		if (condition.tested_property() == lang::official_condition_property::class_) {
			if (!condition.allows_item_class(item_class_name)) {
				diagnostics.push_error_autogen_incompatible_condition(autogen_origin, condition.origin(), item_class_name);
				return false;
			}
		}
		else {
			diagnostics.push_error_autogen_forbidden_condition(autogen_origin, condition.origin());
			return false;
		}

		return true;
	}
};

struct poe1_gem_condition_verifier
{
	[[nodiscard]] bool operator()(
		const lang::official_condition& condition,
		lang::position_tag autogen_origin,
		diagnostics_store& diagnostics) const
	{
		if (condition.tested_property() == lang::official_condition_property::class_) {
			if (!condition.allows_item_class(lang::item_class_names::gems_active)) {
				diagnostics.push_error_autogen_incompatible_condition(autogen_origin, condition.origin(), lang::item_class_names::gems_active);
				return false;
			}

			if (!condition.allows_item_class(lang::item_class_names::gems_support)) {
				diagnostics.push_error_autogen_incompatible_condition(autogen_origin, condition.origin(), lang::item_class_names::gems_support);
				return false;
			}
		}
		else {
			diagnostics.push_error_autogen_forbidden_condition(autogen_origin, condition.origin());
			return false;
		}

		return true;
	}
};

// ---- make_autogen functions ----

template <typename MarketItemType, typename ItemPriceData>
[[nodiscard]] std::function<lang::specific_blocks_generator_func_type<ItemPriceData>> // empty on failure
make_autogen_simple(
	settings st,
	const lang::official_conditions& conditions,
	lang::position_tag autogen_origin,
	std::string_view item_class_name,
	std::vector<MarketItemType> ItemPriceData::* field,
	diagnostics_store& diagnostics)
{
	static_assert(std::is_base_of_v<lang::market::elementary_item, MarketItemType>);

	if (!verify_autogen_conditions(st, conditions, class_condition_verifier{item_class_name}, autogen_origin, diagnostics))
		return {};

	return [field, item_class_name](
			const lang::block_generation_info& block_info,
			const ItemPriceData& item_price_data,
			lang::generated_blocks_consumer consumer)
		{
			return generate_blocks_simple(block_info, item_price_data.*field, item_class_name, consumer);
		};
}

template <typename MarketItemType, typename ItemPriceData>
[[nodiscard]] std::function<lang::specific_blocks_generator_func_type<ItemPriceData>> // empty on failure
make_autogen_stackable_item(
	settings st,
	const lang::official_conditions& conditions,
	lang::position_tag autogen_origin,
	std::string_view item_class_name,
	std::vector<MarketItemType> ItemPriceData::* field,
	const lang::loot::known_items_store& known_items,
	diagnostics_store& diagnostics)
{
	static_assert(std::is_base_of_v<lang::market::elementary_item, MarketItemType>);

	if (!verify_autogen_conditions(st, conditions, class_condition_verifier{item_class_name}, autogen_origin, diagnostics))
		return {};

	return [field, item_class_name, &known_items](
			const lang::block_generation_info& block_info,
			const ItemPriceData& item_price_data,
			lang::generated_blocks_consumer consumer)
		{
			return generate_blocks_stackable_item(block_info, item_price_data.*field, item_class_name, known_items, consumer);
		};
}

[[nodiscard]] std::function<lang::poe1_blocks_generator_func_type> // empty on failure
poe1_make_autogen_gem(
	settings st,
	const lang::official_conditions& conditions,
	lang::position_tag autogen_origin,
	diagnostics_store& diagnostics)
{
	if (!verify_autogen_conditions(st, conditions, poe1_gem_condition_verifier{}, autogen_origin, diagnostics))
		return {};

	return poe1_generate_blocks_gems;
}

[[nodiscard]] std::function<lang::poe1_blocks_generator_func_type> // empty on failure
poe1_make_autogen_func(
	settings st,
	const lang::official_conditions& conditions,
	lang::poe1::autogen_category autogen_category,
	lang::position_tag autogen_origin,
	diagnostics_store& diagnostics)
{
	using cat_t = lang::poe1::autogen_category;
	using ipd = lang::market::poe1::item_price_data;
	namespace cn = lang::item_class_names;

	switch (autogen_category) {
		case cat_t::currency:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::currency_stackable, &ipd::currency, lang::loot::poe1::known_items, diagnostics);
		case cat_t::delirium_orbs:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::delirium_orbs, &ipd::delirium_orbs, lang::loot::poe1::known_items, diagnostics);
		case cat_t::essences:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::essences, &ipd::essences, lang::loot::poe1::known_items, diagnostics);
		case cat_t::fossils:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::fossils, &ipd::fossils, lang::loot::poe1::known_items, diagnostics);
		case cat_t::oils:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::oils, &ipd::oils, lang::loot::poe1::known_items, diagnostics);
		case cat_t::vials:
			return make_autogen_simple(st, conditions, autogen_origin, cn::vials, &ipd::vials, diagnostics);
		case cat_t::fragments:
			return make_autogen_simple(st, conditions, autogen_origin, cn::map_fragments, &ipd::fragments, diagnostics);
		case cat_t::resonators:
			return make_autogen_simple(st, conditions, autogen_origin, cn::resonators, &ipd::resonators, diagnostics);
		case cat_t::scarabs:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::scarabs, &ipd::scarabs, lang::loot::poe1::known_items, diagnostics);
		case cat_t::tattoos:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::tattoos, &ipd::tattoos, lang::loot::poe1::known_items, diagnostics);
		case cat_t::incubators:
			return make_autogen_simple(st, conditions, autogen_origin, cn::incubator, &ipd::incubators, diagnostics);
		case cat_t::cards:
			return make_autogen_simple(st, conditions, autogen_origin, cn::divination_card, &ipd::divination_cards, diagnostics);
		case cat_t::gems:
			return poe1_make_autogen_gem(st, conditions, autogen_origin, diagnostics);

		// NOT IMPLEMENTED - cluster jewels
		// NOT IMPLEMENTED - bases
			// implementation notes
			// - condition: ItemLevel
			// - condition: HasInfluence
			// - print Corrupted False
			// - print Mirrored False
			// - print Rarity Normal Magic Rare
		// NOT IMPLEMENTED - uniques
			// implementation notes
			// - verify Rarity Unique is allowed
			// - print Rarity Unique in generated blocks
			// - detection:
			//   - Class: ? (list all equipment, jewel and flask classes? (to avoid unique maps))
			//   - Corrupted: ?
			// 	 - ShaperItem/ElderItem/HasInfluence: ?
			//   - SynthesisedItem: ?
			//   - AnyEnchantment/HasEnchantment: ? (are there uniques that drop with enchants?)
			//   - LinkedSockets: ?
			//   - AreaLevel: ? (detects boss-specific drops)
			//   - HasSearingExarchImplicit/HasEaterOfWorldsImplicit: ? (detects boss-specific drops)
			//   - Sockets/SocketGroup: ?
			//   - Replica: ?
	}

	diagnostics.push_error_internal_compiler_error(__func__, autogen_origin);
	return {};
}

[[nodiscard]] std::function<lang::poe2_blocks_generator_func_type> // empty on failure
poe2_make_autogen_func(
	settings st,
	const lang::official_conditions& conditions,
	lang::poe2::autogen_category autogen_category,
	lang::position_tag autogen_origin,
	diagnostics_store& diagnostics)
{
	using cat_t = lang::poe2::autogen_category;
	using ipd = lang::market::poe2::item_price_data;
	namespace cn = lang::item_class_names;

	switch (autogen_category) {
		case cat_t::currency:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::currency_stackable, &ipd::currency, lang::loot::poe2::known_items, diagnostics);
		case cat_t::fragments:
			return make_autogen_simple(st, conditions, autogen_origin, cn::map_fragments, &ipd::fragments, diagnostics);
		case cat_t::abyss_currency:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::currency_stackable, &ipd::abyss_currency, lang::loot::poe2::known_items, diagnostics);
		case cat_t::uncut_skill_gems:
			return make_autogen_simple(st, conditions, autogen_origin, cn::poe2::uncut_skill_gems, &ipd::uncut_skill_gems, diagnostics);
		case cat_t::uncut_spirit_gems:
			return make_autogen_simple(st, conditions, autogen_origin, cn::poe2::uncut_spirit_gems, &ipd::uncut_spirit_gems, diagnostics);
		case cat_t::lineage_support_gems:
			return make_autogen_simple(st, conditions, autogen_origin, cn::gems_support, &ipd::lineage_support_gems, diagnostics);
		case cat_t::essences:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::essences, &ipd::essences, lang::loot::poe2::known_items, diagnostics);
		case cat_t::soul_cores:
			return make_autogen_simple(st, conditions, autogen_origin, cn::poe2::soul_cores, &ipd::soul_cores, diagnostics);
		case cat_t::talismans:
			return make_autogen_simple(st, conditions, autogen_origin, cn::poe2::idols, &ipd::talismans, diagnostics);
		case cat_t::runes:
			return make_autogen_simple(st, conditions, autogen_origin, cn::poe2::runes, &ipd::runes, diagnostics);
		case cat_t::omens:
			return make_autogen_simple(st, conditions, autogen_origin, cn::poe2::omens, &ipd::omens, diagnostics);
		case cat_t::expedition:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::currency_stackable, &ipd::expedition, lang::loot::poe2::known_items, diagnostics);
		case cat_t::emotions:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::poe2::emotions, &ipd::emotions, lang::loot::poe2::known_items, diagnostics);
		case cat_t::catalysts:
			return make_autogen_stackable_item(st, conditions, autogen_origin, cn::poe2::catalysts, &ipd::catalysts, lang::loot::poe2::known_items, diagnostics);
	}

	diagnostics.push_error_internal_compiler_error(__func__, autogen_origin);
	return {};
}

template <typename ItemPriceData>
[[nodiscard]] std::function<lang::blocks_generator_func_type> // empty on failure
enclose_autogen_func(std::function<lang::specific_blocks_generator_func_type<ItemPriceData>> func)
{
	return [f = std::move(func)](
		const lang::block_generation_info& info,
		const lang::market::item_price_data& data,
		lang::generated_blocks_consumer consumer)
	{
		FS_ASSERT_MSG(
			std::holds_alternative<ItemPriceData>(data),
			"Implementation bug - autogen/item_price_data PoE 1/2 mismatch!");

		return f(info, std::get<ItemPriceData>(data), consumer);
	};
}

} // namespace

[[nodiscard]] std::optional<lang::autogen_extension>
make_autogen_extension(
	settings st,
	const lang::official_conditions& conditions,
	lang::price_range_condition price_range,
	autogen_protocondition autogen,
	lang::position_tag block_origin,
	diagnostics_store& diagnostics)
{
	if (!price_range.has_bound()) {
		diagnostics.push_message(make_warning(
			diagnostic_message_id::autogen_without_price,
			block_origin,
			"Autogen condition without a price - all known items of specified category will be used"));
		diagnostics.push_message(make_note_minor(autogen.origin, "Autogen specified here"));
	}

	lang::autogen_extension::functions_container_type functions;
	functions.reserve(autogen.categories.size());

	for (lang::autogen_category cat : autogen.categories) {
		std::function<lang::blocks_generator_func_type> func = std::visit(utility::visitor{
			[&](lang::poe1::autogen_category cat) {
				return enclose_autogen_func(poe1_make_autogen_func(st, conditions, cat, autogen.origin, diagnostics));
			},
			[&](lang::poe2::autogen_category cat) {
				return enclose_autogen_func(poe2_make_autogen_func(st, conditions, cat, autogen.origin, diagnostics));
			}
		}, cat);

		// If the user specified autogeneration, func creation should succeed.
		// Otherwise the entire block is invalid and thus nullopt is returned.
		if (!func)
			return std::nullopt;

		functions.push_back(std::move(func));
	}

	return lang::autogen_extension{std::move(functions), price_range, autogen.origin};;
}

}
