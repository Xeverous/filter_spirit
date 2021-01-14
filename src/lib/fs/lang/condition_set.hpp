#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/influence_info.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/condition_properties.hpp>
#include <fs/lang/queries.hpp>
#include <fs/utility/type_traits.hpp>

#include <boost/container/small_vector.hpp>

#include <optional>
#include <vector>
#include <type_traits>

namespace fs::lang
{

template <typename T>
struct range_bound
{
	// used in the implementation
	// T should be a lang type
	static_assert(traits::has_origin_member_v<T>);
	static_assert(traits::has_value_member_v<T>);

	T value;
	bool inclusive;
	position_tag origin;
};

template <typename T> constexpr
bool operator==(range_bound<T> left, range_bound<T> right) noexcept
{
	return left.value == right.value && left.inclusive == right.inclusive;
}

template <typename T> constexpr
bool operator!=(range_bound<T> left, range_bound<T> right) noexcept
{
	return !(left == right);
}

template <typename T>
struct range_condition
{
	using value_type = T;

	constexpr range_condition() = default;
	constexpr range_condition(T val, position_tag origin)
	{
		set_exact(val, origin);
	}

	constexpr bool is_exact() const noexcept
	{
		if (lower_bound.has_value() && upper_bound.has_value())
			return *lower_bound == *upper_bound;

		return false;
	}

	template <typename V>
	constexpr bool test_lower_bound(V value) const noexcept
	{
		static_assert(std::is_same_v<V, decltype(T::value)>);

		if (lower_bound.has_value()) {
			if ((*lower_bound).value.value > value)
				return false;

			if ((*lower_bound).value.value == value && !(*lower_bound).inclusive)
				return false;
		}

		return true;
	}

	template <typename V>
	constexpr bool test_upper_bound(V value) const noexcept
	{
		static_assert(std::is_same_v<V, decltype(T::value)>);

		if (upper_bound.has_value()) {
			if ((*upper_bound).value.value < value)
				return false;

			if ((*upper_bound).value.value == value && !(*upper_bound).inclusive)
				return false;
		}

		return true;
	}

	// check whether 'value' can fit into currently specified range
	template <typename V>
	constexpr bool includes(V value) const noexcept
	{
		return test_lower_bound(value) && test_upper_bound(value);
	}

	constexpr bool has_bound() const noexcept
	{
		return lower_bound.has_value() || upper_bound.has_value();
	}

	constexpr void set_exact(T value, position_tag origin) noexcept
	{
		lower_bound = upper_bound = range_bound<T>{value, true, origin};
	}

	constexpr void set_lower_bound(T value, bool inclusive, position_tag origin) noexcept
	{
		lower_bound = range_bound<T>{value, inclusive, origin};
	}

	constexpr void set_upper_bound(T value, bool inclusive, position_tag origin) noexcept
	{
		upper_bound = range_bound<T>{value, inclusive, origin};
	}

	constexpr std::optional<lang::position_tag> first_origin() const noexcept
	{
		if (lower_bound.has_value() && upper_bound.has_value()) {
			if (compare((*lower_bound).origin, (*upper_bound).origin) < 0)
				return (*lower_bound).origin;
			else
				return (*upper_bound).origin;
		}
		else if (lower_bound.has_value()) {
			return (*lower_bound).origin;
		}
		else if (upper_bound.has_value()) {
			return (*upper_bound).origin;
		}
		else {
			return std::nullopt;
		}
	}

	constexpr std::optional<lang::position_tag> second_origin() const noexcept
	{
		if (lower_bound.has_value() && upper_bound.has_value()) {
			if (compare((*lower_bound).origin, (*upper_bound).origin) < 0)
				return (*upper_bound).origin;
			else
				return (*lower_bound).origin;
		}
		else {
			return std::nullopt;
		}
	}

	std::optional<range_bound<T>> lower_bound;
	std::optional<range_bound<T>> upper_bound;
};

using rarity_range_condition = range_condition<rarity>;
using integer_range_condition = range_condition<integer>;
using fractional_range_condition = range_condition<fractional>;

struct boolean_condition
{
	boolean value;
	position_tag origin;
};

struct gem_quality_type_condition
{
	using container_type = boost::container::small_vector<gem_quality_type, 4>;

	container_type values;
	position_tag origin;
};

struct socket_spec_condition
{
	using container_type = boost::container::small_vector<socket_spec, 6>;

	comparison_type comparison;
	container_type values;
	position_tag origin;
};

struct strings_condition
{
	std::vector<lang::string> strings;
	bool exact_match_required;
	position_tag origin;
};

struct influences_condition
{
	influence_info influence;
	bool exact_match_required;
	position_tag origin;
};

struct condition_set
{
	void generate(std::ostream& output_stream) const;

	/**
	 * @brief determines whether given condition set is valid
	 * for generation into the actual filter
	 *
	 * @details if a string condition (eg base type) is an empty array
	 * the resulting filter would not be valid - PoE client will not
	 * accept base type conditions that do not specify any base name
	 */
	[[nodiscard]]
	bool is_valid() const;

	integer_range_condition item_level;
	integer_range_condition drop_level;
	integer_range_condition quality;
	rarity_range_condition rarity;
	std::optional<strings_condition> class_;
	std::optional<strings_condition> base_type;
	integer_range_condition linked_sockets;
	std::optional<socket_spec_condition> sockets;
	std::optional<socket_spec_condition> socket_group;
	integer_range_condition height;
	integer_range_condition width;
	std::optional<strings_condition> has_explicit_mod;
	std::optional<strings_condition> has_enchantment;
	std::optional<strings_condition> prophecy;
	std::optional<strings_condition> enchantment_passive_node;
	std::optional<influences_condition> has_influence;
	std::optional<gem_quality_type_condition> gem_quality_type;
	integer_range_condition stack_size;
	integer_range_condition gem_level;
	integer_range_condition map_tier;
	integer_range_condition area_level;
	integer_range_condition corrupted_mods;
	integer_range_condition enchantment_passive_num;
	std::optional<boolean_condition> is_identified;
	std::optional<boolean_condition> is_corrupted;
	std::optional<boolean_condition> is_mirrored;
	std::optional<boolean_condition> is_elder_item;
	std::optional<boolean_condition> is_shaper_item;
	std::optional<boolean_condition> is_fractured_item;
	std::optional<boolean_condition> is_synthesised_item;
	std::optional<boolean_condition> is_enchanted;
	std::optional<boolean_condition> is_shaped_map;
	std::optional<boolean_condition> is_elder_map;
	std::optional<boolean_condition> is_blighted_map;
	std::optional<boolean_condition> is_replica;
	std::optional<boolean_condition> is_alternate_quality;
};

class condition_match_result
{
public:
	condition_match_result(
		bool is_successful,
		lang::position_tag condition_origin,
		std::optional<lang::position_tag> value_origin)
	: _is_successful(is_successful)
	, _condition_origin(condition_origin)
	, _value_origin(value_origin)
	{
	}

	static condition_match_result success(
		lang::position_tag condition_origin,
		std::optional<lang::position_tag> value_origin = std::nullopt)
	{
		return condition_match_result(true, condition_origin, value_origin);
	}

	static condition_match_result failure(
		lang::position_tag condition_origin,
		std::optional<lang::position_tag> value_origin = std::nullopt)
	{
		return condition_match_result(false, condition_origin, value_origin);
	}

	bool is_successful() const
	{
		return _is_successful;
	}

	lang::position_tag condition_origin() const
	{
		return _condition_origin;
	}

	std::optional<lang::position_tag> value_origin() const
	{
		return _value_origin;
	}

private:
	bool _is_successful;
	lang::position_tag _condition_origin;
	std::optional<lang::position_tag> _value_origin;
};

struct range_condition_match_result
{
	std::optional<condition_match_result> lower_bound;
	std::optional<condition_match_result> upper_bound;
};

struct condition_set_match_result
{
	bool is_successful() const
	{
		const auto is_not_failure = [](const std::optional<condition_match_result>& result) {
			if (result)
				return (*result).is_successful();
			else
				return true; // match not attempted => not considered as failure
		};

		const auto is_not_failure_range = [&](const range_condition_match_result& result) {
			return is_not_failure(result.lower_bound) && is_not_failure(result.upper_bound);
		};

		return is_not_failure_range(item_level)
			&& is_not_failure_range(drop_level)
			&& is_not_failure_range(quality)
			&& is_not_failure_range(rarity)
			&& is_not_failure_range(linked_sockets)
			&& is_not_failure_range(height)
			&& is_not_failure_range(width)
			&& is_not_failure_range(stack_size)
			&& is_not_failure_range(gem_level)
			&& is_not_failure_range(map_tier)
			&& is_not_failure_range(area_level)
			&& is_not_failure_range(corrupted_mods)
			&& is_not_failure_range(enchantment_passive_num)
			&& is_not_failure(class_)
			&& is_not_failure(base_type)
			&& is_not_failure(sockets)
			&& is_not_failure(socket_group)
			&& is_not_failure(has_explicit_mod)
			&& is_not_failure(has_enchantment)
			&& is_not_failure(prophecy)
			&& is_not_failure(enchantment_passive_node)
			&& is_not_failure(has_influence)
			&& is_not_failure(gem_quality_type)
			&& is_not_failure(is_identified)
			&& is_not_failure(is_corrupted)
			&& is_not_failure(is_mirrored)
			&& is_not_failure(is_elder_item)
			&& is_not_failure(is_shaper_item)
			&& is_not_failure(is_fractured_item)
			&& is_not_failure(is_synthesised_item)
			&& is_not_failure(is_enchanted)
			&& is_not_failure(is_shaped_map)
			&& is_not_failure(is_elder_map)
			&& is_not_failure(is_blighted_map)
			&& is_not_failure(is_replica)
			&& is_not_failure(is_alternate_quality);
	}

	range_condition_match_result item_level;
	range_condition_match_result drop_level;
	range_condition_match_result quality;
	range_condition_match_result rarity;
	range_condition_match_result linked_sockets;
	range_condition_match_result height;
	range_condition_match_result width;
	range_condition_match_result stack_size;
	range_condition_match_result gem_level;
	range_condition_match_result map_tier;
	range_condition_match_result area_level;
	range_condition_match_result corrupted_mods;
	range_condition_match_result enchantment_passive_num;
	std::optional<condition_match_result> class_;
	std::optional<condition_match_result> base_type;
	std::optional<condition_match_result> sockets;
	std::optional<condition_match_result> socket_group;
	std::optional<condition_match_result> has_explicit_mod;
	std::optional<condition_match_result> has_enchantment;
	std::optional<condition_match_result> prophecy;
	std::optional<condition_match_result> enchantment_passive_node;
	std::optional<condition_match_result> has_influence;
	std::optional<condition_match_result> gem_quality_type;
	std::optional<condition_match_result> is_identified;
	std::optional<condition_match_result> is_corrupted;
	std::optional<condition_match_result> is_mirrored;
	std::optional<condition_match_result> is_elder_item;
	std::optional<condition_match_result> is_shaper_item;
	std::optional<condition_match_result> is_fractured_item;
	std::optional<condition_match_result> is_synthesised_item;
	std::optional<condition_match_result> is_enchanted;
	std::optional<condition_match_result> is_shaped_map;
	std::optional<condition_match_result> is_elder_map;
	std::optional<condition_match_result> is_blighted_map;
	std::optional<condition_match_result> is_replica;
	std::optional<condition_match_result> is_alternate_quality;
};

struct autogen_condition
{
	item_category category;
	position_tag origin;
};

struct spirit_condition_set
{
	condition_set conditions;

	// spirit filter extensions
	fractional_range_condition price;
	std::optional<autogen_condition> autogen;
};

}
