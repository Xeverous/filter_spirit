#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/detail/autogen.hpp>
#include <fs/lang/conditions.hpp>
#include <fs/lang/item.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/limits.hpp>
#include <fs/lang/market/item_price_data.hpp>

#include <string_view>
#include <initializer_list>
#include <type_traits>

namespace fs::compiler::detail {

namespace {

// ---- item search/match functions ----

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
		if (item.price.is_low_confidence || !price_range.includes(item.price.chaos_value))
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

struct gem_matcher
{
	int level;
	int quality;
	bool corrupt;

	bool operator()(const lang::market::gem& g) const
	{
		return g.level == level && g.quality == quality && g.is_corrupted == corrupt;
	}
};

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

	block.conditions.conditions.push_back(lang::make_class_condition(
		lang::equality_comparison_type::exact_match,
		lang::condition_values_container<lang::string>{
			lang::string{std::string(item_class_name), block_info.autogen_origin}
		},
		block_info.autogen_origin));
	block.conditions.conditions.push_back(lang::make_base_type_condition(
		lang::equality_comparison_type::exact_match,
		get_matching_items(item_price_data_field, empty_matcher{}, block_info.price_range, block_info.autogen_origin),
		block_info.autogen_origin));

	if (block.conditions.is_valid())
		consumer.push(std::move(block));
}

void generate_blocks_gems(
	const lang::block_generation_info& block_info,
	const lang::market::item_price_data& item_price_data,
	lang::generated_blocks_consumer consumer)
{
	for (int level = lang::limits::min_item_gem_level; level <= lang::limits::max_item_gem_level; ++level) {
		for (int quality = lang::limits::min_item_gem_quality; quality <= lang::limits::max_item_gem_quality; ++quality) {
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
						gem_matcher{level, quality, corrupt},
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

struct gem_condition_verifier
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

template <typename MarketItemType>
[[nodiscard]] std::function<lang::blocks_generator_func_type>
make_autogen_simple(
	settings st,
	const lang::official_conditions& conditions,
	lang::position_tag autogen_origin,
	std::string_view item_class_name,
	std::vector<MarketItemType> lang::market::item_price_data::* field,
	diagnostics_store& diagnostics)
{
	static_assert(std::is_base_of_v<lang::market::elementary_item, MarketItemType>);

	if (!verify_autogen_conditions(st, conditions, class_condition_verifier{item_class_name}, autogen_origin, diagnostics))
		return {};

	return [field, item_class_name](
			const lang::block_generation_info& block_info,
			const lang::market::item_price_data& item_price_data,
			lang::generated_blocks_consumer consumer)
		{
			return generate_blocks_simple(block_info, item_price_data.*field, item_class_name, consumer);
		};
}

[[nodiscard]] std::function<lang::blocks_generator_func_type>
make_autogen_gem(
	settings st,
	const lang::official_conditions& conditions,
	lang::position_tag autogen_origin,
	diagnostics_store& diagnostics)
{
	if (!verify_autogen_conditions(st, conditions, gem_condition_verifier{}, autogen_origin, diagnostics))
		return {};

	return generate_blocks_gems;
}

} // namespace

[[nodiscard]] std::function<lang::blocks_generator_func_type>
make_autogen_func(
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

	using ipd = lang::market::item_price_data;
	namespace cn = lang::item_class_names;

	switch (autogen.category) {
		// TODO some of these may benefit from additional StackSize condition
		case lang::autogen_category::currency:
			return make_autogen_simple(st, conditions, autogen.origin, cn::currency_stackable, &ipd::currency, diagnostics);
		case lang::autogen_category::delirium_orbs:
			return make_autogen_simple(st, conditions, autogen.origin, cn::delirium_orbs, &ipd::delirium_orbs, diagnostics);
		case lang::autogen_category::essences:
			return make_autogen_simple(st, conditions, autogen.origin, cn::essences, &ipd::essences, diagnostics);
		case lang::autogen_category::fossils:
			return make_autogen_simple(st, conditions, autogen.origin, cn::fossils, &ipd::fossils, diagnostics);
		case lang::autogen_category::oils:
			return make_autogen_simple(st, conditions, autogen.origin, cn::oils, &ipd::oils, diagnostics);
		case lang::autogen_category::vials:
			return make_autogen_simple(st, conditions, autogen.origin, cn::vials, &ipd::vials, diagnostics);
		case lang::autogen_category::fragments:
			return make_autogen_simple(st, conditions, autogen.origin, cn::map_fragments, &ipd::fragments, diagnostics);
		case lang::autogen_category::resonators:
			return make_autogen_simple(st, conditions, autogen.origin, cn::resonators, &ipd::resonators, diagnostics);
		case lang::autogen_category::scarabs:
			return make_autogen_simple(st, conditions, autogen.origin, cn::scarabs, &ipd::scarabs, diagnostics);
		case lang::autogen_category::incubators:
			return make_autogen_simple(st, conditions, autogen.origin, cn::incubator, &ipd::vials, diagnostics);
		case lang::autogen_category::cards:
			return make_autogen_simple(st, conditions, autogen.origin, cn::divination_card, &ipd::divination_cards, diagnostics);
		case lang::autogen_category::gems:
			return make_autogen_gem(st, conditions, autogen.origin, diagnostics);

		case lang::autogen_category::bases:
			// implementation notes
			// - condition: ItemLevel
			// - condition: HasInfluence
			// - print Corrupted False
			// - print Mirrored False
			// - print Rarity Normal Magic Rare
		case lang::autogen_category::uniques_eq_unambiguous:
		case lang::autogen_category::uniques_eq_ambiguous:
		case lang::autogen_category::uniques_flasks_unambiguous:
		case lang::autogen_category::uniques_flasks_ambiguous:
		case lang::autogen_category::uniques_jewels_unambiguous:
		case lang::autogen_category::uniques_jewels_ambiguous:
		case lang::autogen_category::uniques_maps_unambiguous:
		case lang::autogen_category::uniques_maps_ambiguous:
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
			diagnostics.push_message(make_warning(diagnostic_message_id::minor_note, autogen.origin, "not implemented"));
			return [](
				const lang::block_generation_info& /* block_info */,
				const lang::market::item_price_data& /* item_price_data */,
				lang::generated_blocks_consumer /* consumer */)
			{};
	}

	diagnostics.push_error_internal_compiler_error(__func__, autogen.origin);
	return {};
}

}
