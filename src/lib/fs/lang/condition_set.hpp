#pragma once

#include <fs/lang/primitive_types.hpp>
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

	// check whether 'value' can fit into currently specified range
	template <typename V>
	constexpr bool includes(V value) const noexcept
	{
		static_assert(std::is_same_v<V, decltype(T::value)>);

		if (lower_bound.has_value())
		{
			if ((*lower_bound).value.value > value)
				return false;

			if ((*lower_bound).value.value == value && !(*lower_bound).inclusive)
				return false;
		}

		if (upper_bound.has_value())
		{
			if ((*upper_bound).value.value < value)
				return false;

			if ((*upper_bound).value.value == value && !(*upper_bound).inclusive)
				return false;
		}

		return true;
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

struct socket_spec_condition
{
	using container_type = boost::container::small_vector<socket_spec, 6>;

	comparison_type comparison;
	container_type values;
	position_tag origin;
};

struct strings_condition
{
	std::vector<std::string> strings;
	bool exact_match_required;
	position_tag origin;
};

struct influences_condition
{
	bool is_none() const
	{
		return !(shaper || elder || crusader || redeemer || hunter || warlord);
	}

	bool shaper;
	bool elder;
	bool crusader;
	bool redeemer;
	bool hunter;
	bool warlord;
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
	integer_range_condition links;
	std::optional<socket_spec_condition> sockets;
	std::optional<socket_spec_condition> socket_group;
	integer_range_condition height;
	integer_range_condition width;
	std::optional<strings_condition> has_explicit_mod;
	std::optional<strings_condition> has_enchantment;
	std::optional<strings_condition> prophecy;
	std::optional<strings_condition> enchantment_passive_node;
	std::optional<influences_condition> has_influence;
	integer_range_condition stack_size;
	integer_range_condition gem_level;
	integer_range_condition map_tier;
	integer_range_condition area_level;
	integer_range_condition corrupted_mods;
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
