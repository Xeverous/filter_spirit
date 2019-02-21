#pragma once
#include "lang/types.hpp"
#include <memory>
#include <variant>
#include <vector>
#include <iosfwd>

namespace fs::lang
{

template <typename T>
struct range_bound
{
	T value;
	bool inclusive;
};

enum class range_relation
{
	identical, superset, subset, intersect, disjoint
};

template <typename T>
class range_condition
{
public:
	range_condition(comparison_type comparison, T value)
	{
		if (comparison == comparison_type::greater)
		{
			lower_bound = {value, false};
		}
		else if (comparison == comparison_type::greater_equal)
		{
			lower_bound = {value, true};
		}
		else if (comparison == comparison_type::less)
		{
			upper_bound = {value, false};
		}
		else if (comparison == comparison_type::less_equal)
		{
			upper_bound = {value, true};
		}
		else
		{
			lower_bound = {value, true};
			upper_bound = {value, true};
		}
	}

	[[nodiscard]] static
	range_relation relation_first_to_second(range_condition first, range_condition second)
	{
		bound_relation lower_bound_relation = get_lower_bound_relation(first.lower_bound, second.lower_bound, second.upper_bound);
		bound_relation upper_bound_relation = get_upper_bound_relation(first.upper_bound, second.lower_bound, second.upper_bound);

		if (lower_bound_relation == bound_relation::disjoint ||
			upper_bound_relation == bound_relation::disjoint)
			return range_relation::disjoint;

		if (lower_bound_relation == bound_relation::more_restrictive &&
			upper_bound_relation == bound_relation::more_restrictive)
			return range_relation::subset;

		if (lower_bound_relation == bound_relation::identical &&
			upper_bound_relation == bound_relation::more_restrictive)
			return range_relation::subset;

		if (lower_bound_relation == bound_relation::more_restrictive &&
			upper_bound_relation == bound_relation::identical)
			return range_relation::subset;

		if (lower_bound_relation == bound_relation::less_restrictive &&
			upper_bound_relation == bound_relation::less_restrictive)
			return range_relation::superset;

		if (lower_bound_relation == bound_relation::identical &&
			upper_bound_relation == bound_relation::less_restrictive)
			return range_relation::superset;

		if (lower_bound_relation == bound_relation::less_restrictive &&
			upper_bound_relation == bound_relation::identical)
			return range_relation::superset;

		if (lower_bound_relation == bound_relation::identical &&
			upper_bound_relation == bound_relation::identical)
			return range_relation::identical;

		return range_relation::intersect;
	}

	std::optional<range_bound<T>> lower_bound;
	std::optional<range_bound<T>> upper_bound;

private:
	enum class bound_relation { identical, more_restrictive, less_restrictive, disjoint };

	[[nodiscard]] static
	bound_relation get_lower_bound_relation(
		std::optional<range_bound<T>> lower_bound,
		std::optional<range_bound<T>> other_lower_bound,
		std::optional<range_bound<T>> other_upper_bound)
	{
		if (lower_bound.has_value() && other_upper_bound.has_value())
		{
			 if ((*lower_bound).value > (*other_upper_bound).value)
				 return bound_relation::disjoint;

			 if ((*lower_bound).value == (*other_upper_bound).value)
			 {
				 if (!(*lower_bound).inclusive || !(*other_upper_bound).inclusive)
					 return bound_relation::disjoint;
			 }
		}

		if (lower_bound.has_value())
		{
			if (other_lower_bound.has_value())
			{
				if ((*lower_bound).value < (*other_lower_bound).value)
					return bound_relation::less_restrictive;

				if ((*lower_bound).value > (*other_lower_bound).value)
					return bound_relation::more_restrictive;

				if ((*lower_bound).inclusive && !(*other_lower_bound).inclusive)
					return bound_relation::less_restrictive;

				if (!(*lower_bound).inclusive && (*other_lower_bound).inclusive)
					return bound_relation::more_restrictive;

				return bound_relation::identical;
			}
			else
			{
				return bound_relation::more_restrictive;
			}
		}
		else
		{
			if (other_lower_bound.has_value())
			{
				return bound_relation::less_restrictive;
			}
			else
			{
				return bound_relation::identical;
			}
		}
	}

	[[nodiscard]] static
	bound_relation get_upper_bound_relation(
		std::optional<range_bound<T>> upper_bound,
		std::optional<range_bound<T>> other_lower_bound,
		std::optional<range_bound<T>> other_upper_bound)
	{
		if (upper_bound.has_value() && other_lower_bound.has_value())
		{
			 if ((*upper_bound).value < (*other_lower_bound).value)
				 return bound_relation::disjoint;

			 if ((*upper_bound).value == (*other_lower_bound).value)
			 {
				 if (!(*upper_bound).inclusive || !(*other_lower_bound).inclusive)
					 return bound_relation::disjoint;
			 }
		}

		if (upper_bound.has_value())
		{
			if (other_lower_bound.has_value())
			{
				if ((*upper_bound).value < (*other_lower_bound).value)
					return bound_relation::more_restrictive;

				if ((*upper_bound).value > (*other_lower_bound).value)
					return bound_relation::less_restrictive;

				if ((*upper_bound).inclusive && !(*other_lower_bound).inclusive)
					return bound_relation::less_restrictive;

				if (!(*upper_bound).inclusive && (*other_lower_bound).inclusive)
					return bound_relation::more_restrictive;

				return bound_relation::identical;
			}
			else
			{
				return bound_relation::more_restrictive;
			}
		}
		else
		{
			if (other_lower_bound.has_value())
			{
				return bound_relation::less_restrictive;
			}
			else
			{
				return bound_relation::identical;
			}
		}
	}
};

using numeric_range_condition = range_condition<int>;
using rarity_range_condition = range_condition<rarity>;

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

	std::optional<numeric_range_condition> item_level;
	std::optional<numeric_range_condition> drop_level;
	std::optional<numeric_range_condition> quality;
	std::optional<rarity_range_condition> rarity;
	std::shared_ptr<std::vector<std::string>> class_;
	std::shared_ptr<std::vector<std::string>> base_type;
	std::optional<numeric_range_condition> sockets;
	std::optional<numeric_range_condition> links;
	std::optional<socket_group> socket_group;
	std::optional<numeric_range_condition> height;
	std::optional<numeric_range_condition> width;
	std::shared_ptr<std::vector<std::string>> has_explicit_mod;
	std::optional<numeric_range_condition> stack_size;
	std::optional<numeric_range_condition> gem_level;
	std::optional<numeric_range_condition> map_tier;
	std::optional<boolean> is_identified;
	std::optional<boolean> is_corrupted;
	std::optional<boolean> is_shaper_item;
	std::optional<boolean> is_elder_item;
	std::optional<boolean> is_shaped_map;
};

}
