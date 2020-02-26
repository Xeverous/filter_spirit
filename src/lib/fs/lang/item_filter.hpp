#pragma once
#include <fs/lang/condition_set.hpp>
#include <fs/lang/action_set.hpp>

#include <iosfwd>
#include <optional>
#include <vector>

namespace fs::lang
{

struct item_visibility
{
	bool show;
	position_tag origin;
};

struct item_filter_block
{
	void generate(std::ostream& output_stream) const;

	item_visibility visibility;
	condition_set conditions;
	action_set actions;
};

struct autogen_extension
{
	fractional_range_condition price_range;
	autogen_condition condition;
};

struct spirit_item_filter_block
{
	item_filter_block block;

	// spirit filter extensions
	std::optional<autogen_extension> autogen;
};

/**
 * @class a core type representing item filter
 *
 * @details This is a result of the parsers and a type
 * used in various debugging functionalities
 */
struct item_filter
{
	std::vector<item_filter_block> blocks;
};

struct spirit_item_filter
{
	std::vector<spirit_item_filter_block> blocks;
};

}
