#include <fs/lang/position_tag.hpp>
#include <fs/lang/primitive_types.hpp>
#include <fs/lang/conditions.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/string_helpers.hpp>

#include <algorithm>
#include <numeric>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace fs::lang
{

namespace
{

std::ostream& operator<<(std::ostream& os, boolean b)
{
	return os << ' ' << to_string_view(b.value);
}

std::ostream& operator<<(std::ostream& os, rarity r)
{
	return os << ' ' << to_string_view(r.value);
}

std::ostream& operator<<(std::ostream& os, integer n)
{
	return os << ' ' << n.value;
}

std::ostream& operator<<(std::ostream& os, const string& str)
{
	return os << " \"" << str.value << '\"';
}

std::ostream& operator<<(std::ostream& os, influence_spec spec)
{
	if (spec.is_none()) {
		os << ' ' << keywords::rf::none;
	}
	else {
		if (spec.shaper)
			os << ' ' << keywords::rf::shaper;

		if (spec.elder)
			os << ' ' << keywords::rf::elder;

		if (spec.crusader)
			os << ' ' << keywords::rf::crusader;

		if (spec.redeemer)
			os << ' ' << keywords::rf::redeemer;

		if (spec.hunter)
			os << ' ' << keywords::rf::hunter;

		if (spec.warlord)
			os << ' ' << keywords::rf::warlord;
	}

	return os;
}

std::ostream& operator<<(std::ostream& os, socket_spec ss)
{
	const auto output_letter = [&](char letter, int times) {
		for (int i = 0; i < times; ++i)
			os << letter;
	};

	os << ' ';

	FS_ASSERT(ss.is_valid());

	if (ss.num.has_value())
		os << *ss.num;

	output_letter(keywords::rf::r, ss.r);
	output_letter(keywords::rf::g, ss.g);
	output_letter(keywords::rf::b, ss.b);
	output_letter(keywords::rf::w, ss.w);
	output_letter(keywords::rf::a, ss.a);
	output_letter(keywords::rf::d, ss.d);

	return os;
}

template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const boost::container::small_vector<T, N>& values)
{
	for (const T& value : values)
		os << value;

	return os;
}

template <typename T>
void print_condition(official_condition_property property, comparison_type cmp, std::optional<integer> count, const T& value, std::ostream& os)
{
	const auto cmp_sv = to_string_view(cmp);
	os << '\t' << to_keyword(property) << (cmp_sv.empty() ? "" : " ") << cmp_sv;
	if (count)
		os << count->value;
	// note: individual values (potentially multiple) are expected to print spaces before each element
	os /* << ' ' */ << value << '\n'; // intentionally no space
}

template <typename T>
void print_condition(official_condition_property property, comparison_type cmp, const T& value, std::ostream& os)
{
	print_condition(property, cmp, std::nullopt, std::move(value), os);
}

template <typename Comparable>
bool compare_values(comparison_type cmp_type, Comparable lhs, Comparable rhs)
{
	// strings should not be compared by this function because:
	// - they have to implement singular vs plural compatibility ("Map" == "Maps")
	// - they have to implement character compatibility ("Maelstrom" == "Maelström")
	static_assert(!std::is_same_v<Comparable, string>);
	static_assert(!std::is_same_v<Comparable, std::string>);

	switch (cmp_type) {
		case comparison_type::less:
			return lhs < rhs;
		case comparison_type::less_equal:
			return lhs <= rhs;
		case comparison_type::equal:
		case comparison_type::exact_match:
			return lhs == rhs;
		case comparison_type::greater:
			return lhs > rhs;
		case comparison_type::greater_equal:
			return lhs >= rhs;
		case comparison_type::not_equal:
			return lhs != rhs;
	}

	return false; // C++23: use std::unreachable
}

const string* find_match(
	std::string_view sv,
	const condition_values_container<string>& values,
	bool exact_match_required)
{
	for (const auto& value : values) {
		if (utility::compare_strings_ignore_diacritics(sv, value.value, exact_match_required))
			return &value;
	}

	return nullptr;
}

template <typename Container>
int count_string_matches(std::string_view sv, const Container& values, bool exact_match_required)
{
	using value_type = typename Container::value_type;
	return static_cast<int>(std::count_if(values.begin(), values.end(), [&](const value_type& value) {
		return utility::compare_strings_ignore_diacritics(sv, value, exact_match_required);
	}));
}

/*
 * Sockets and SocketGroup are one of the most unintuitive and complex conditions.
 * There are multiple inconsistencies:
 * - items with no sockets get special treatment, as if they had one link-group of size 0
 *   (which is different from having no link-groups)
 * - an item can match < but not <=
 * - an item can match > but not >=
 * - < and > use OR operation for subrequirements, other comparisons use AND (function C)
 *   (thus "< 5G" is equivalent to "< 5 G" but "<= 5G" is not equivalent to "<= 5 G")
 * - soft-equal (no comparison symbol or only 1 "=" character):
 *   - uses == to compare sockets/link count (function A)
 *   - uses >= for color counts (function B)
 *   - (in other words, link count must match exactly but colors may exceed requirements)
 *
 * Sockets     <  5GGG # less than 5        sockets OR  less than 3x G socket
 * Sockets     <= 5GGG # at most   5        sockets AND at most   3x G socket
 * Sockets        5GGG # exactly   5        sockets AND at least  3x G socket
 * Sockets     == 5GGG # exactly   5        sockets AND exactly   3x G socket
 * Sockets     >= 5GGG # at least  5        sockets AND at least  3x G socket
 * Sockets     >  5GGG # more than 5        sockets OR  more than 3x G socket
 * SocketGroup <  5GGG # less than 5 linked sockets OR  less than 3x G linked within any  group
 * SocketGroup <= 5GGG # at most   5 linked sockets AND at most   3x G linked within such group
 * SocketGroup    5GGG # exactly   5 linked sockets AND at least  3x G linked within such group
 * SocketGroup == 5GGG # exactly   5 linked sockets AND exactly   3x G linked within such group
 * SocketGroup >= 5GGG # at least  5 linked sockets AND at least  3x G linked within such group
 * SocketGroup >  5GGG # more than 5 linked sockets OR  more than 3x G linked within any  group
 *
 * != works as expected: works as nothing/= but expects 0 matches instead of at least 1.
 */
[[nodiscard]] bool // (A)
test_socket_or_link_count(std::optional<int> opt_req, comparison_type comparison, int item_stat)
{
	if (!opt_req)
		return true;

	return compare_values(comparison, item_stat, *opt_req);
}

[[nodiscard]] std::optional<bool> // (B)
test_color_count(int req, comparison_type comparison, int item_stat)
{
	/*
	 * It is impossible to query for count == 0 of specific socket color.
	 * This is because for each required specific color, a specific letter
	 * needs to be written. Multiple letters increase required count.
	 * No letters means no requirement therefore the caller must check
	 * for results elsewhere. There is no good default here, return nothing.
	 */
	if (req == 0)
		return std::nullopt;

	switch (comparison) {
		case comparison_type::less:
			return item_stat < req;
		case comparison_type::less_equal:
			return item_stat <= req;
		case comparison_type::greater:
			return item_stat > req;
		case comparison_type::greater_equal:
		case comparison_type::equal:
			return item_stat >= req; // yes, this is how it works
		case comparison_type::exact_match:
			return item_stat == req;
		case comparison_type::not_equal:
			break; // not tested
	}

	return std::nullopt;
}

[[nodiscard]] bool
test_color_count_all_groups(const socket_info& item_sockets, socket_color c, int req, comparison_type comparison)
{
	for (const linked_sockets& group : item_sockets.groups) {
		if (test_color_count(req, comparison, group.count_of(c)).value_or(false))
			return true;
	}

	return false;
}

[[nodiscard]] bool // (C)
is_sockets_condition_using_or(comparison_type comparison)
{
	return comparison == comparison_type::less || comparison == comparison_type::greater;
}

// group_matters: false for Sockets, true for SocketGroup
// comparison: should not be != because:
// 1) Correct implementation is not just another switch case.
// 2) For positive conditions, at least one match is required but
//    for negative conditions, all matches must fail.
// Thus, a proper implementation handles != on a higher abstraction layer.
[[nodiscard]] bool
test_sockets_condition(
	comparison_type comparison,
	socket_spec ss,
	bool group_matters,
	const socket_info& item_sockets)
{
	FS_ASSERT(comparison != comparison_type::not_equal);

	if (item_sockets.groups.empty()) {
		return test_socket_or_link_count(ss.num, comparison, 0)
			&& test_color_count(ss.r, comparison, 0).value_or(true)
			&& test_color_count(ss.g, comparison, 0).value_or(true)
			&& test_color_count(ss.b, comparison, 0).value_or(true)
			&& test_color_count(ss.w, comparison, 0).value_or(true)
			&& test_color_count(ss.a, comparison, 0).value_or(true)
			&& test_color_count(ss.d, comparison, 0).value_or(true);
	}

	if (group_matters) {
		if (is_sockets_condition_using_or(comparison)) {
			return test_socket_or_link_count(ss.num, comparison, item_sockets.links())
				|| test_color_count_all_groups(item_sockets, socket_color::r, ss.r, comparison)
				|| test_color_count_all_groups(item_sockets, socket_color::g, ss.g, comparison)
				|| test_color_count_all_groups(item_sockets, socket_color::b, ss.b, comparison)
				|| test_color_count_all_groups(item_sockets, socket_color::w, ss.w, comparison)
				|| test_color_count_all_groups(item_sockets, socket_color::a, ss.a, comparison)
				|| test_color_count_all_groups(item_sockets, socket_color::d, ss.d, comparison);
		}
		else {
			for (const linked_sockets& group : item_sockets.groups) {
				const bool result = test_socket_or_link_count(ss.num, comparison, static_cast<int>(group.sockets.size()))
					&& test_color_count(ss.r, comparison, group.count_of(socket_color::r)).value_or(true)
					&& test_color_count(ss.g, comparison, group.count_of(socket_color::g)).value_or(true)
					&& test_color_count(ss.b, comparison, group.count_of(socket_color::b)).value_or(true)
					&& test_color_count(ss.w, comparison, group.count_of(socket_color::w)).value_or(true)
					&& test_color_count(ss.a, comparison, group.count_of(socket_color::a)).value_or(true)
					&& test_color_count(ss.d, comparison, group.count_of(socket_color::d)).value_or(true);

				if (result)
					return true;
			}

			return false;
		}
	}
	else {
		if (is_sockets_condition_using_or(comparison)) {
			return test_socket_or_link_count(ss.num, comparison, item_sockets.sockets())
				|| test_color_count(ss.r, comparison, item_sockets.count_of(socket_color::r)).value_or(false)
				|| test_color_count(ss.g, comparison, item_sockets.count_of(socket_color::g)).value_or(false)
				|| test_color_count(ss.b, comparison, item_sockets.count_of(socket_color::b)).value_or(false)
				|| test_color_count(ss.w, comparison, item_sockets.count_of(socket_color::w)).value_or(false)
				|| test_color_count(ss.a, comparison, item_sockets.count_of(socket_color::a)).value_or(false)
				|| test_color_count(ss.d, comparison, item_sockets.count_of(socket_color::d)).value_or(false);
		}
		else {
			return test_socket_or_link_count(ss.num, comparison, item_sockets.sockets())
				&& test_color_count(ss.r, comparison, item_sockets.count_of(socket_color::r)).value_or(true)
				&& test_color_count(ss.g, comparison, item_sockets.count_of(socket_color::g)).value_or(true)
				&& test_color_count(ss.b, comparison, item_sockets.count_of(socket_color::b)).value_or(true)
				&& test_color_count(ss.w, comparison, item_sockets.count_of(socket_color::w)).value_or(true)
				&& test_color_count(ss.a, comparison, item_sockets.count_of(socket_color::a)).value_or(true)
				&& test_color_count(ss.d, comparison, item_sockets.count_of(socket_color::d)).value_or(true);
		}
	}
}

} // namespace

void boolean_condition::print(std::ostream& os) const
{
	print_condition(tested_property(), comparison_type::equal, value(), os);
}

condition_match_result has_influence_condition::test_item(const item& itm, int /* area_level */) const
{
	/*
	 * HasInfluence condition is a bit different.
	 * 1) HasInfluence None is an official feature and will match items with exactly no influence
	 * 2) It behaves differently with ==:
	 *    - If there is == (exact matching), it will only match items with all specified influences.
	 *    - If there is nothing or =, it will match an item with at least one of specified influences.
	 */

	// Various cases could return origin of "None" or influence keywords but currently
	// the implementation does not store origins of "None" or these keywords.

	if (m_influence_spec.is_none()) {
		if (itm.influence.is_none())
			return condition_match_result::success(origin());
		else
			return condition_match_result::failure(origin());
	}

	if (m_exact_match) {
		if (m_influence_spec.shaper   && !itm.influence.shaper)
			return condition_match_result::failure(origin());

		if (m_influence_spec.elder    && !itm.influence.elder)
			return condition_match_result::failure(origin());

		if (m_influence_spec.crusader && !itm.influence.crusader)
			return condition_match_result::failure(origin());

		if (m_influence_spec.redeemer && !itm.influence.redeemer)
			return condition_match_result::failure(origin());

		if (m_influence_spec.hunter   && !itm.influence.hunter)
			return condition_match_result::failure(origin());

		if (m_influence_spec.warlord  && !itm.influence.warlord)
			return condition_match_result::failure(origin());

		return condition_match_result::success(origin());
	}
	else {
		if (m_influence_spec.shaper   && itm.influence.shaper)
			return condition_match_result::success(origin());

		if (m_influence_spec.elder    && itm.influence.elder)
			return condition_match_result::success(origin());

		if (m_influence_spec.crusader && itm.influence.crusader)
			return condition_match_result::success(origin());

		if (m_influence_spec.redeemer && itm.influence.redeemer)
			return condition_match_result::success(origin());

		if (m_influence_spec.hunter   && itm.influence.hunter)
			return condition_match_result::success(origin());

		if (m_influence_spec.warlord  && itm.influence.warlord)
			return condition_match_result::success(origin());

		return condition_match_result::failure(origin());
	}
}

void has_influence_condition::print(std::ostream& os) const
{
	print_condition(
		tested_property(),
		m_exact_match ? comparison_type::exact_match : comparison_type::equal,
		m_influence_spec,
		os);
}

void range_bound_condition_base::print_impl(comparison_type comparison, rarity bound_value, std::ostream& os) const
{
	print_condition(tested_property(), comparison, bound_value, os);
}

void range_bound_condition_base::print_impl(comparison_type comparison, integer bound_value, std::ostream& os) const
{
	print_condition(tested_property(), comparison, bound_value, os);
}

void value_list_condition_base::print_impl(bool allowed, const condition_values_container<rarity>& values, std::ostream& os) const
{
	print_condition(tested_property(), allowed ? comparison_type::equal : comparison_type::not_equal, values, os);
}

void value_list_condition_base::print_impl(bool allowed, const condition_values_container<integer>& values, std::ostream& os) const
{
	print_condition(tested_property(), allowed ? comparison_type::equal : comparison_type::not_equal, values, os);
}

bool string_comparison_condition::allows_item_class(std::string_view class_name) const
{
	if (tested_property() != official_condition_property::class_)
		return true;

	return find_match(class_name, m_values, m_comparison_type == equality_comparison_type::exact_match) != nullptr;
}

condition_match_result string_comparison_condition::test_item(const item& itm, int /* area_level */) const
{
	const std::string* const item_field = get_item_field(itm);
	const string* match = nullptr;

	if (item_field != nullptr) {
		match = find_match(
			*item_field, m_values, m_comparison_type == equality_comparison_type::exact_match);
	}

	const bool success = (match == nullptr) == (m_comparison_type == equality_comparison_type::not_equal);
	return condition_match_result(
		success, origin(), match == nullptr ? std::optional<position_tag>() : match->origin);
}

void string_comparison_condition::print(std::ostream& os) const
{
	print_condition(tested_property(), to_comparison_type(m_comparison_type), m_values, os);
}

void counted_string_comparison_condition::print(std::ostream& os) const
{
	print_condition(tested_property(), m_comparison_type, m_count, m_values, os);
}

condition_match_result counted_string_comparison_condition::test_item(const item& itm, int /* area_level */) const
{
	const auto matches = count_matches(itm, m_values, m_comparison_type == comparison_type::exact_match);
	if (m_count) {
		const bool success = compare_values(m_comparison_type, matches, (*m_count).value);
		return condition_match_result(success, origin());
	}
	else {
		return condition_match_result(matches > 0, origin());
	}
}

int has_explicit_mod_condition::count_matches(
	const item& itm, const counted_string_comparison_condition::container_type& values, bool exact_match_required) const
{
	return std::accumulate(values.begin(), values.end(), 0, [&](int result, const string& str) {
		return result + count_string_matches(str.value, itm.explicit_mods, exact_match_required);
	});
}

int has_enchantment_condition::count_matches(
	const item& itm, const counted_string_comparison_condition::container_type& values, bool exact_match_required) const
{
	if (!itm.enchantment_labyrinth)
		return 0;

	return static_cast<int>(std::count_if(values.begin(), values.end(), [&](const string& str) {
		return utility::compare_strings_ignore_diacritics(str.value, *itm.enchantment_labyrinth, exact_match_required);
	}));
}

void socket_specification_condition::print(std::ostream& os) const
{
	print_condition(tested_property(), m_comparison_type, m_values, os);
}

condition_match_result socket_specification_condition::test_item(const item& itm, int /* area_level */) const
{
	const bool group_matters = tested_property() == official_condition_property::socket_group;
	const bool is_negative = m_comparison_type == comparison_type::not_equal;
	const auto cmp = is_negative ? comparison_type::equal : m_comparison_type;

	const auto it = std::find_if(m_values.begin(), m_values.end(), [&](socket_spec ss) {
		return test_sockets_condition(cmp, ss, group_matters, itm.sockets);
	});

	const bool is_successful = is_negative == (it == m_values.end());
	const auto value_origin = (it != m_values.end()) ? it->origin : std::optional<position_tag>();
	return condition_match_result{is_successful, origin(), value_origin};
}

}
