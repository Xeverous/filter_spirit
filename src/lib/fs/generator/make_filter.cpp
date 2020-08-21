#include <fs/generator/make_filter.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/condition_set.hpp>
#include <fs/lang/item_classes.hpp>
#include <fs/utility/assert.hpp>

#include <string>
#include <utility>
#include <type_traits>

namespace
{

using namespace fs;

// C++20: add concepts
template <
	typename ForwardIterator,
	typename Predicate, // bool (ForwardIterator), decides whether item matches
	typename NameExtractor> // std::string (ForwardIerator), returns item name
[[nodiscard]] std::vector<std::string>
get_matching_items(
	ForwardIterator first,
	ForwardIterator last,
	Predicate pred,
	NameExtractor name)
{
	static_assert(
		std::is_same_v<bool, std::invoke_result_t<Predicate, decltype(*first)>>,
		"Predicate should accept iterator and return bool");
	static_assert(
		std::is_same_v<std::string, std::invoke_result_t<NameExtractor, decltype(*first)>>,
		"NameExtractor should accept iterator and return std::string");

	std::vector<std::string> result;

	for (auto it = first; it != last; ++it) {
		if (pred(*it))
			result.push_back(name(*it));
	}

	return result;
}

template <
	typename ForwardIterator,
	typename Predicate,
	typename NameExtractor>
[[nodiscard]] lang::strings_condition
make_strings_condition(
	ForwardIterator first,
	ForwardIterator last,
	Predicate pred,
	NameExtractor name,
	lang::position_tag autogen_origin)
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
		get_matching_items(first, last, pred, name),
		true,
		autogen_origin};
}

template <typename ForwardIterator>
[[nodiscard]] lang::strings_condition
make_strings_condition_basic(
	ForwardIterator first,
	ForwardIterator last,
	lang::fractional_range_condition price_range,
	lang::position_tag autogen_origin)
{
	return make_strings_condition(
		first,
		last,
		[&](const lang::market::elementary_item& item) {
			return price_range.includes(item.price.chaos_value);
		},
		[&](const lang::market::elementary_item& item) {
			return item.name;
		},
		autogen_origin);
}

template <typename ForwardIterator>
[[nodiscard]] lang::strings_condition
make_strings_condition_uniques_unambiguous(
	ForwardIterator first,
	ForwardIterator last,
	lang::fractional_range_condition price_range,
	lang::position_tag autogen_origin)
{
	return make_strings_condition(
		first,
		last,
		[price_range](const auto& pair) {
			const lang::market::elementary_item& item = pair.second;
			return price_range.includes(item.price.chaos_value);
		},
		[](const auto& pair) {
			return pair.first;
		},
		autogen_origin);
}

template <typename ForwardIterator>
[[nodiscard]] lang::strings_condition
make_strings_condition_uniques_ambiguous(
	ForwardIterator first,
	ForwardIterator last,
	lang::fractional_range_condition price_range,
	lang::position_tag autogen_origin)
{
	return make_strings_condition(
		first,
		last,
		[price_range](const auto& pair) {
			const auto& items = pair.second; // C++20: use concept container
			BOOST_ASSERT(!items.empty());

			for (const lang::market::elementary_item& item : items)
				if (price_range.includes(item.price.chaos_value))
					return true;
			return false;
		},
		[](const auto& pair) {
			return pair.first;
		},
		autogen_origin);
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
	switch (autogen.condition.category) {
		case lang::item_category::cards:
			result_block.conditions.base_type = make_strings_condition_basic(
				item_price_data.divination_cards.begin(),
				item_price_data.divination_cards.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.class_ = lang::strings_condition{
				{fs::lang::classes::cards},
				true,
				autogen.condition.origin};
			break;
		case lang::item_category::prophecies:
			result_block.conditions.prophecy = make_strings_condition_basic(
				item_price_data.prophecies.begin(),
				item_price_data.prophecies.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.class_ = lang::strings_condition{
				{fs::lang::classes::prophecies},
				true,
				autogen.condition.origin};
			break;
		case lang::item_category::essences:
			result_block.conditions.base_type = make_strings_condition_basic(
				item_price_data.essences.begin(),
				item_price_data.essences.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.class_ = lang::strings_condition{
				{fs::lang::classes::essences},
				true,
				autogen.condition.origin};
			break;
		case lang::item_category::fossils:
			result_block.conditions.base_type = make_strings_condition_basic(
				item_price_data.fossils.begin(),
				item_price_data.fossils.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.class_ = lang::strings_condition{
				{fs::lang::classes::fossils},
				true,
				autogen.condition.origin};
			break;
		case lang::item_category::resonators:
			result_block.conditions.base_type = make_strings_condition_basic(
				item_price_data.resonators.begin(),
				item_price_data.resonators.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.class_ = lang::strings_condition{
				{fs::lang::classes::resonators},
				true,
				autogen.condition.origin};
			break;
		case lang::item_category::scarabs:
			result_block.conditions.base_type = make_strings_condition_basic(
				item_price_data.scarabs.begin(),
				item_price_data.scarabs.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.class_ = lang::strings_condition{
				{fs::lang::classes::scarabs},
				true,
				autogen.condition.origin};
			break;
		case lang::item_category::incubators:
			result_block.conditions.base_type = make_strings_condition_basic(
				item_price_data.incubators.begin(),
				item_price_data.incubators.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.class_ = lang::strings_condition{
				{fs::lang::classes::incubators},
				true,
				autogen.condition.origin};
			break;
		case lang::item_category::oils:
			result_block.conditions.base_type = make_strings_condition_basic(
				item_price_data.oils.begin(),
				item_price_data.oils.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.class_ = lang::strings_condition{
				{fs::lang::classes::oils},
				true,
				autogen.condition.origin};
			break;
		case lang::item_category::catalysts:
			result_block.conditions.base_type = make_strings_condition_basic(
				item_price_data.catalysts.begin(),
				item_price_data.catalysts.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.class_ = lang::strings_condition{
				{fs::lang::classes::catalysts},
				true,
				autogen.condition.origin};
			break;
		case lang::item_category::uniques_eq_unambiguous:
			result_block.conditions.base_type = make_strings_condition_uniques_unambiguous(
				item_price_data.unique_eq.unambiguous.begin(),
				item_price_data.unique_eq.unambiguous.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.rarity = lang::rarity_range_condition(
				lang::rarity{lang::rarity_type::unique, autogen.condition.origin},
				autogen.condition.origin);
			break;
		case lang::item_category::uniques_eq_ambiguous:
			result_block.conditions.base_type = make_strings_condition_uniques_ambiguous(
				item_price_data.unique_eq.ambiguous.begin(),
				item_price_data.unique_eq.ambiguous.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.rarity = lang::rarity_range_condition(
				lang::rarity{lang::rarity_type::unique, autogen.condition.origin},
				autogen.condition.origin);
			break;
		case lang::item_category::uniques_flasks_unambiguous:
			result_block.conditions.base_type = make_strings_condition_uniques_unambiguous(
				item_price_data.unique_flasks.unambiguous.begin(),
				item_price_data.unique_flasks.unambiguous.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.rarity = lang::rarity_range_condition(
				lang::rarity{lang::rarity_type::unique, autogen.condition.origin},
				autogen.condition.origin);
			break;
		case lang::item_category::uniques_flasks_ambiguous:
			result_block.conditions.base_type = make_strings_condition_uniques_ambiguous(
				item_price_data.unique_flasks.ambiguous.begin(),
				item_price_data.unique_flasks.ambiguous.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.rarity = lang::rarity_range_condition(
				lang::rarity{lang::rarity_type::unique, autogen.condition.origin},
				autogen.condition.origin);
			break;
		case lang::item_category::uniques_jewels_unambiguous:
			result_block.conditions.base_type = make_strings_condition_uniques_unambiguous(
				item_price_data.unique_jewels.unambiguous.begin(),
				item_price_data.unique_jewels.unambiguous.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.rarity = lang::rarity_range_condition(
				lang::rarity{lang::rarity_type::unique, autogen.condition.origin},
				autogen.condition.origin);
			break;
		case lang::item_category::uniques_jewels_ambiguous:
			result_block.conditions.base_type = make_strings_condition_uniques_ambiguous(
				item_price_data.unique_jewels.ambiguous.begin(),
				item_price_data.unique_jewels.ambiguous.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.rarity = lang::rarity_range_condition(
				lang::rarity{lang::rarity_type::unique, autogen.condition.origin},
				autogen.condition.origin);
			break;
		// TODO maps should use tier condition to filter results
		case lang::item_category::uniques_maps_unambiguous:
			result_block.conditions.base_type = make_strings_condition_uniques_unambiguous(
				item_price_data.unique_maps.unambiguous.begin(),
				item_price_data.unique_maps.unambiguous.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.rarity = lang::rarity_range_condition(
				lang::rarity{lang::rarity_type::unique, autogen.condition.origin},
				autogen.condition.origin);
			break;
		case lang::item_category::uniques_maps_ambiguous:
			result_block.conditions.base_type = make_strings_condition_uniques_ambiguous(
				item_price_data.unique_maps.ambiguous.begin(),
				item_price_data.unique_maps.ambiguous.end(),
				autogen.price_range,
				autogen.condition.origin);
			result_block.conditions.rarity = lang::rarity_range_condition(
				lang::rarity{lang::rarity_type::unique, autogen.condition.origin},
				autogen.condition.origin);
			break;
	}

	return result_block;
}

} // namespace

namespace fs::generator
{

lang::item_filter
make_filter(
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
