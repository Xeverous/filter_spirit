#include <fs/generator/make_item_filter.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/condition_set.hpp>
#include <fs/lang/item.hpp>
#include <fs/utility/assert.hpp>

#include <string>
#include <utility>
#include <type_traits>
#include <stdexcept>

namespace
{

using namespace fs;

// C++20: add concepts
template <
	typename ForwardIterator,
	typename Predicate,     // bool (ForwardIterator)
	typename NameExtractor> // const std::string& (ForwardIterator)
[[nodiscard]] std::vector<lang::string>
get_matching_items(
	ForwardIterator first, // first item from the API
	ForwardIterator last,  // last item from the API
	Predicate pred,        // decides whether item matches price (and any other conditions)
	NameExtractor name,    // returns item name
	const std::optional<lang::strings_condition>& opt_base_type) // filters matches
{
	static_assert(
		std::is_same_v<bool, std::invoke_result_t<Predicate, decltype(*first)>>,
		"Predicate should accept iterator and return bool");
	static_assert(
		std::is_same_v<const std::string&, std::invoke_result_t<NameExtractor, decltype(*first)>>,
		"NameExtractor should accept iterator and return const std::string&");

	auto is_name_allowed = [&](const std::string& name) {
		if (!opt_base_type)
			return true;

		return (*opt_base_type).find_match(name) != nullptr;
	};

	std::vector<lang::string> result;
	for (auto it = first; it != last; ++it) {
		if (pred(*it) && is_name_allowed(name(*it)))
			result.push_back(lang::string{name(*it), {}}); // TODO add autogen origin when supported
	}

	return result;
}

template <
	typename ForwardIterator,
	typename Predicate,
	typename NameExtractor>
[[nodiscard]] lang::strings_condition
make_strings_condition(
	lang::position_tag autogen_origin,
	ForwardIterator first,
	ForwardIterator last,
	Predicate pred,
	NameExtractor name,
	const std::optional<lang::strings_condition>& opt_base_type)
{
	/*
	 * 1. We set exact matching to true (== token in filters) because
	 * all of generated data contains full names. We never want to match
	 * name substring and having no exact match can result in extra
	 * unwanted matches when one full name is a substring of another.
	 *
	 * 2. This block is synthesised from an extension so it does not
	 * have any "real" origin in the source file. The best thing we
	 * can do is to set origin to point to autogeneration.
	 */
	return lang::strings_condition{
		get_matching_items(first, last, pred, name, opt_base_type),
		true,
		autogen_origin};
}

template <typename Container>
[[nodiscard]] lang::strings_condition
make_strings_condition_basic(
	lang::position_tag autogen_origin,
	const Container& items,
	lang::fractional_range_condition price_range,
	const std::optional<lang::strings_condition>& opt_base_type)
{
	static_assert(
		std::is_base_of_v<
			lang::market::elementary_item,
			typename Container::value_type>,
		"items container should store elementary_item or derived type");

	return make_strings_condition(
		autogen_origin,
		items.begin(),
		items.end(),
		[price_range](const lang::market::elementary_item& item) {
			return price_range.includes(item.price.chaos_value) && !item.price.is_low_confidence;
		},
		[](const lang::market::elementary_item& item) -> const std::string& {
			return item.name;
		},
		opt_base_type);
}

[[nodiscard]] lang::strings_condition
make_strings_condition_gems(
	lang::position_tag autogen_origin,
	const std::vector<lang::market::gem>& gems,
	lang::fractional_range_condition price_range,
	const std::optional<lang::strings_condition>& opt_base_type,
	lang::integer_range_condition gem_level,
	lang::integer_range_condition quality,
	std::optional<lang::boolean_condition> opt_corrupted)
{
	if (!gem_level.has_bound())
		throw std::runtime_error("missing GemLevel condition for gems autogeneration");

	if (!quality.has_bound())
		throw std::runtime_error("missing Quality condition for gems autogeneration");

	if (!opt_corrupted)
		throw std::runtime_error("missing Corrupted condition for gems autogeneration");

	return make_strings_condition(
		autogen_origin,
		gems.begin(),
		gems.end(),
		[&](const lang::market::gem& gem) {
			return
				   price_range.includes(gem.price.chaos_value)
				&& gem_level.includes(gem.level)
				&& quality.includes(gem.quality)
				&& (*opt_corrupted).value.value == gem.is_corrupted
				&& !gem.price.is_low_confidence;
		},
		[](const lang::market::elementary_item& item) -> const std::string& {
			return item.name;
		},
		opt_base_type);
}

[[nodiscard]] lang::strings_condition
make_strings_condition_bases(
	lang::position_tag autogen_origin,
	const std::vector<lang::market::base>& bases,
	lang::fractional_range_condition price_range,
	const std::optional<lang::strings_condition>& opt_base_type,
	lang::integer_range_condition item_level,
	const std::optional<lang::influences_condition>& opt_has_influence)
{
	if (!item_level.has_bound())
		throw std::runtime_error("missing ItemLevel condition for bases autogeneration");

	if (!opt_has_influence)
		throw std::runtime_error("missing HasInfluence condition for bases autogeneration");

	if (!(*opt_has_influence).exact_match_required)
		throw std::runtime_error("HasInfluence condition for bases autogeneration should require exact matching (\"==\")");

	return make_strings_condition(
		autogen_origin,
		bases.begin(),
		bases.end(),
		[&](const lang::market::base& base) {
			return
				   price_range.includes(base.price.chaos_value)
				&& item_level.includes(base.item_level)
				&& (*opt_has_influence).influence == base.influence
				&& !base.price.is_low_confidence;
		},
		[](const lang::market::elementary_item& item) -> const std::string& {
			return item.name;
		},
		opt_base_type);
}

[[nodiscard]] lang::strings_condition
make_strings_condition_uniques_unambiguous(
	lang::position_tag autogen_origin,
	const lang::market::unique_item_price_data::unambiguous_container_type& items,
	lang::fractional_range_condition price_range,
	const std::optional<lang::strings_condition>& opt_base_type)
{
	return make_strings_condition(
		autogen_origin,
		items.begin(),
		items.end(),
		[price_range](const auto& pair) {
			const lang::market::elementary_item& item = pair.second;
			return price_range.includes(item.price.chaos_value) && !item.price.is_low_confidence;
		},
		[](const auto& pair) -> const std::string& {
			return pair.first;
		},
		opt_base_type);
}

[[nodiscard]] lang::strings_condition
make_strings_condition_uniques_ambiguous(
	lang::position_tag autogen_origin,
	const lang::market::unique_item_price_data::ambiguous_container_type& items,
	lang::fractional_range_condition price_range,
	const std::optional<lang::strings_condition>& opt_base_type)
{
	return make_strings_condition(
		autogen_origin,
		items.begin(),
		items.end(),
		[price_range](const auto& pair) {
			const auto& items = pair.second; // C++20: use concept container
			FS_ASSERT_MSG(
				items.size() >= 2u,
				"If uniques are ambiguous on specific base then there should be at least 2 of them.");

			for (const lang::market::elementary_item& item : items)
				if (price_range.includes(item.price.chaos_value) && !item.price.is_low_confidence)
					return true;
			return false;
		},
		[](const auto& pair) -> const std::string& {
			return pair.first;
		},
		opt_base_type);
}

[[nodiscard]] lang::item_filter_block
make_filter_block(
	const lang::spirit_item_filter_block& block_template,
	const lang::market::item_price_data& item_price_data)
{
	if (!block_template.autogen)
		return block_template.block;

	lang::item_filter_block result_block = block_template.block;

	const lang::autogen_extension& autogen = *block_template.autogen;

	auto make_condition_base_type_basic = [&](const auto& items_container) {
		return make_strings_condition_basic(
			autogen.condition.origin,
			items_container,
			autogen.price_range,
			result_block.conditions.base_type);
	};

	auto make_condition_base_type_unique_unambiguous =
		[&](const lang::market::unique_item_price_data::unambiguous_container_type& items)
	{
		return make_strings_condition_uniques_unambiguous(
			autogen.condition.origin,
			items,
			autogen.price_range,
			result_block.conditions.base_type);
	};

	auto make_condition_base_type_unique_ambiguous =
		[&](const lang::market::unique_item_price_data::ambiguous_container_type& items)
	{
		return make_strings_condition_uniques_ambiguous(
			autogen.condition.origin,
			items,
			autogen.price_range,
			result_block.conditions.base_type);
	};

	// TODO when better origins are supported, add autogen origin
	auto make_condition_class = [origin = autogen.condition.origin](std::string_view class_name) {
		return lang::strings_condition{
			{lang::string{std::string(class_name), origin}},
			true,
			origin};
	};

	auto make_condition_rarity_unique = [origin = autogen.condition.origin]() {
		return lang::rarity_range_condition(lang::rarity{lang::rarity_type::unique, origin}, origin);
	};

	auto make_condition_boolean_false = [origin = autogen.condition.origin]() {
		return lang::boolean_condition{lang::boolean{false, origin}, origin};
	};

	switch (autogen.condition.category) {
		namespace cn = lang::item_class_names;

		case lang::item_category::currency:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.currency);
			result_block.conditions.class_ = make_condition_class(cn::currency_stackable);
			break;
		case lang::item_category::fragments:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.fragments);
			result_block.conditions.class_ = make_condition_class(cn::map_fragments);
			break;
		case lang::item_category::delirium_orbs:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.delirium_orbs);
			result_block.conditions.class_ = make_condition_class(cn::delirium_orbs);
			break;
		case lang::item_category::cards:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.divination_cards);
			result_block.conditions.class_ = make_condition_class(cn::divination_card);
			break;
		case lang::item_category::prophecies:
			result_block.conditions.prophecy = make_condition_base_type_basic(item_price_data.prophecies);
			result_block.conditions.class_ = make_condition_class(cn::prophecies);
			break;
		case lang::item_category::essences:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.essences);
			result_block.conditions.class_ = make_condition_class(cn::essences);
			break;
		case lang::item_category::fossils:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.fossils);
			result_block.conditions.class_ = make_condition_class(cn::fossils);
			break;
		case lang::item_category::resonators:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.resonators);
			result_block.conditions.class_ = make_condition_class(cn::resonators);
			break;
		case lang::item_category::scarabs:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.scarabs);
			result_block.conditions.class_ = make_condition_class(cn::scarabs);
			break;
		case lang::item_category::incubators:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.incubators);
			result_block.conditions.class_ = make_condition_class(cn::incubator);
			break;
		case lang::item_category::oils:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.oils);
			result_block.conditions.class_ = make_condition_class(cn::oils);
			break;
		case lang::item_category::vials:
			result_block.conditions.base_type = make_condition_base_type_basic(item_price_data.vials);
			result_block.conditions.class_ = make_condition_class(cn::vials);
			break;
		case lang::item_category::gems:
			result_block.conditions.base_type = make_strings_condition_gems(
				autogen.condition.origin,
				item_price_data.gems,
				autogen.price_range,
				result_block.conditions.base_type,
				result_block.conditions.gem_level,
				result_block.conditions.quality,
				result_block.conditions.is_corrupted);
			if (!result_block.conditions.class_) {
				result_block.conditions.class_ = lang::strings_condition{
					{
						lang::string{std::string(cn::gems_active),  autogen.condition.origin},
						lang::string{std::string(cn::gems_support), autogen.condition.origin}
					},
					true,
					autogen.condition.origin};
			}
			break;
		case lang::item_category::bases:
			result_block.conditions.base_type = make_strings_condition_bases(
				autogen.condition.origin,
				item_price_data.bases,
				autogen.price_range,
				result_block.conditions.base_type,
				result_block.conditions.item_level,
				result_block.conditions.has_influence);
			result_block.conditions.is_mirrored = make_condition_boolean_false();
			result_block.conditions.is_corrupted = make_condition_boolean_false();
			if (!result_block.conditions.rarity.has_bound()) {
				result_block.conditions.rarity.set_lower_bound(
					fs::lang::rarity{lang::rarity_type::normal, autogen.condition.origin}, true, autogen.condition.origin);
				result_block.conditions.rarity.set_upper_bound(
					fs::lang::rarity{lang::rarity_type::rare, autogen.condition.origin}, true, autogen.condition.origin);
			}
			break;
		case lang::item_category::uniques_eq_unambiguous:
			result_block.conditions.base_type = make_condition_base_type_unique_unambiguous(
				item_price_data.unique_eq.unambiguous);
			result_block.conditions.rarity = make_condition_rarity_unique();
			break;
		case lang::item_category::uniques_eq_ambiguous:
			result_block.conditions.base_type = make_condition_base_type_unique_ambiguous(
				item_price_data.unique_eq.ambiguous);
			result_block.conditions.rarity = make_condition_rarity_unique();
			break;
		case lang::item_category::uniques_flasks_unambiguous:
			result_block.conditions.base_type = make_condition_base_type_unique_unambiguous(
				item_price_data.unique_flasks.unambiguous);
			result_block.conditions.rarity = make_condition_rarity_unique();
			break;
		case lang::item_category::uniques_flasks_ambiguous:
			result_block.conditions.base_type = make_condition_base_type_unique_ambiguous(
				item_price_data.unique_flasks.ambiguous);
			result_block.conditions.rarity = make_condition_rarity_unique();
			break;
		case lang::item_category::uniques_jewels_unambiguous:
			result_block.conditions.base_type = make_condition_base_type_unique_unambiguous(
				item_price_data.unique_jewels.unambiguous);
			result_block.conditions.rarity = make_condition_rarity_unique();
			break;
		case lang::item_category::uniques_jewels_ambiguous:
			result_block.conditions.base_type = make_condition_base_type_unique_ambiguous(
				item_price_data.unique_jewels.ambiguous);
			result_block.conditions.rarity = make_condition_rarity_unique();
			break;
		// TODO maps should use tier condition to filter results
		case lang::item_category::uniques_maps_unambiguous:
			result_block.conditions.base_type = make_condition_base_type_unique_unambiguous(
				item_price_data.unique_maps.unambiguous);
			result_block.conditions.rarity = make_condition_rarity_unique();
			break;
		case lang::item_category::uniques_maps_ambiguous:
			result_block.conditions.base_type = make_condition_base_type_unique_ambiguous(
				item_price_data.unique_maps.ambiguous);
			result_block.conditions.rarity = make_condition_rarity_unique();
			break;
	}

	return result_block;
}

} // namespace

namespace fs::generator
{

lang::item_filter
make_item_filter(
	const lang::spirit_item_filter& filter_template,
	const lang::market::item_price_data& item_price_data)
{
	std::vector<lang::item_filter_block> blocks;
	blocks.reserve(filter_template.blocks.size());

	for (const auto& b : filter_template.blocks) {
		lang::item_filter_block block = make_filter_block(b, item_price_data);
		if (block.conditions.is_valid())
			blocks.push_back(std::move(block));
	}

	return lang::item_filter{std::move(blocks)};
}

}
