#pragma once

#include <fs/lang/object.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace fs::lang
{

template <typename T>
struct range_bound
{
	T value;
	bool inclusive;
	position_tag origin;
};

template <typename T>
bool operator==(range_bound<T> left, range_bound<T> right)
{
	return left.value == right.value && left.inclusive == right.inclusive;
}

template <typename T>
bool operator!=(range_bound<T> left, range_bound<T> right)
{
	return !(left == right);
}

template <typename T>
struct range_condition
{
	bool is_exact() const
	{
		if (lower_bound.has_value() && upper_bound.has_value())
			return *lower_bound == *upper_bound;

		return false;
	}

	// check whether 'value' can fit into currently specified range
	bool includes(T value) const
	{
		if (lower_bound.has_value())
		{
			if ((*lower_bound).value > value)
				return false;

			if ((*lower_bound).value == value && !(*lower_bound).inclusive)
				return false;
		}

		if (upper_bound.has_value())
		{
			if ((*upper_bound).value < value)
				return false;

			if ((*upper_bound).value == value && !(*upper_bound).inclusive)
				return false;
		}

		return true;
	}

	bool has_anything() const
	{
		return lower_bound.has_value() || upper_bound.has_value();
	}

	void set_exact(T value, position_tag origin)
	{
		lower_bound = upper_bound = range_bound<T>{value, true, origin};
	}

	void set_lower_bound(T value, bool inclusive, position_tag origin)
	{
		lower_bound = range_bound<T>{value, inclusive, origin};
	}

	void set_upper_bound(T value, bool inclusive, position_tag origin)
	{
		upper_bound = range_bound<T>{value, inclusive, origin};
	}

	std::optional<range_bound<T>> lower_bound;
	std::optional<range_bound<T>> upper_bound;
};

using numeric_range_condition = range_condition<int>;
using rarity_range_condition = range_condition<rarity>;

struct boolean_condition
{
	boolean value;
	position_tag origin;
};

struct socket_group_condition
{
	socket_group group;
	position_tag origin;
};

struct strings_condition
{
	std::shared_ptr<std::vector<std::string>> strings;
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

	numeric_range_condition item_level;
	numeric_range_condition drop_level;
	numeric_range_condition quality;
	rarity_range_condition rarity;
	strings_condition class_;
	strings_condition base_type;
	numeric_range_condition sockets;
	numeric_range_condition links;
	std::optional<socket_group_condition> socket_group;
	numeric_range_condition height;
	numeric_range_condition width;
	strings_condition has_explicit_mod;
	strings_condition has_enchantment;
	numeric_range_condition stack_size;
	numeric_range_condition gem_level;
	numeric_range_condition map_tier;
	std::optional<boolean_condition> is_identified;
	std::optional<boolean_condition> is_corrupted;
	std::optional<boolean_condition> is_elder_item;
	std::optional<boolean_condition> is_shaper_item;
	std::optional<boolean_condition> is_fractured_item;
	std::optional<boolean_condition> is_synthesised_item;
	std::optional<boolean_condition> is_enchanted;
	std::optional<boolean_condition> is_shaped_map;
};

}
