#pragma once

#include <fs/lang/conditions.hpp>
#include <fs/lang/action_set.hpp>

#include <functional>
#include <iosfwd>
#include <iterator>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace fs::lang
{

// ---- block pieces other than conditions and actions ----

struct item;

struct item_visibility
{
	item_visibility_policy policy;
	position_tag origin;
};

struct block_continuation
{
	std::optional<position_tag> origin;
};

enum class block_match_status { success, failure_mismatch, failure_invalid_block, ignored_import_block };

struct block_match_result
{
private:
	// prevent instancing with broken invariants
	block_match_result(
		block_match_status status,
		position_tag block_statement_origin,
		std::optional<position_tag> continue_origin,
		std::vector<condition_match_result> match_results)
	: status(status)
	, block_statement_origin(block_statement_origin)
	, continue_origin(continue_origin)
	, match_results(std::move(match_results))
	{}

public:
	static block_match_result ignored_import_block(
		position_tag block_statement_origin)
	{
		return block_match_result(
			block_match_status::ignored_import_block, block_statement_origin, std::nullopt, {});
	}

	static block_match_result success(
		position_tag block_statement_origin,
		std::optional<position_tag> continue_origin,
		std::vector<condition_match_result> match_results)
	{
		return block_match_result(
			block_match_status::success, block_statement_origin, continue_origin, std::move(match_results));
	}

	static block_match_result failure_mismatch(
		position_tag block_statement_origin,
		std::optional<position_tag> continue_origin,
		std::vector<condition_match_result> match_results)
	{
		return block_match_result(
			block_match_status::failure_mismatch, block_statement_origin, continue_origin, std::move(match_results));
	}

	static block_match_result failure_invalid_block(
		position_tag block_statement_origin,
		std::optional<position_tag> continue_origin)
	{
		return block_match_result(
			block_match_status::failure_invalid_block, block_statement_origin, continue_origin, {});
	}

	bool is_successful() const { return status == block_match_status::success; }

	block_match_status status;
	position_tag block_statement_origin; // Show/Hide/Minimal or Import
	std::optional<position_tag> continue_origin;
	std::vector<condition_match_result> match_results;
};

// ---- real_filter_representation ----

struct import_block
{
	void print(std::ostream& output_stream) const;

	string path;
	bool is_optional;
	position_tag origin;
};

struct item_filter_block
{
	item_filter_block(item_visibility visibility)
	: visibility(visibility)
	{}

	item_filter_block(
		item_visibility visibility,
		official_conditions conditions,
		action_set actions,
		block_continuation continuation)
	: visibility(visibility)
	, conditions(std::move(conditions))
	, actions(std::move(actions))
	, continuation(continuation)
	{}

	bool is_valid() const
	{
		return conditions.is_valid() && visibility.policy != item_visibility_policy::discard;
	}

	block_match_result test_item(const item& itm, int area_level) const;

	void print(std::ostream& output_stream) const;

	item_visibility visibility;
	official_conditions conditions;
	action_set actions;
	block_continuation continuation;
};

using block_variant = std::variant<item_filter_block, import_block>;

struct item_filter
{
	void print(std::ostream& output_stream) const;

	std::vector<block_variant> blocks;
};

// ---- pieces for spirit filter ----

namespace market { struct item_price_data; }

struct block_generation_info
{
	item_visibility visibility;
	action_set actions;
	block_continuation continuation;
	position_tag autogen_origin;
	price_range_condition price_range;
};

struct generated_blocks_consumer
{
	void push(item_filter_block&& block)
	{
		blocks.get().emplace_back(std::move(block));
	}

	std::reference_wrapper<std::vector<block_variant>> blocks;
};

using blocks_generator_func_type = void (
	const block_generation_info&,
	const market::item_price_data&,
	generated_blocks_consumer
);

struct autogen_extension
{
	std::function<blocks_generator_func_type> blocks_generator; // should never be empty
	price_range_condition price_range;
	position_tag origin;
};

// ---- spirit_filter_representation ----

struct spirit_item_filter_block
{
	item_filter_block block;
	std::optional<autogen_extension> autogen;
};

using spirit_block_variant = std::variant<spirit_item_filter_block, import_block>;

struct spirit_item_filter
{
	std::vector<spirit_block_variant> blocks;
};

// ---- types for item filtering ----

struct item_visibility_style
{
	bool show;
	position_tag origin;
};

struct item_style
{
	item_style();

	void override_with(const action_set& actions);

	item_visibility_style visibility;

	color_action text_color;
	std::optional<color_action> border_color;
	color_action background_color;
	font_size_action font_size;
	std::optional<play_effect_action> effect;
	std::optional<minimap_icon_action> minimap_icon;
	std::optional<alert_sound_action> alert_sound;
	std::optional<switch_drop_sound_action> switch_drop_sound;
	std::optional<switch_drop_sound_action> switch_drop_sound_if_alert_sound;
};

struct item_filtering_result
{
	item_style style;

	// Each element represents a match attempt against subsequent filter block.
	// size() may be smaller than filter's size() because filtering stops on first non-continue match.
	std::vector<block_match_result> match_history;
};

item_filtering_result pass_item_through_filter(const item& itm, const item_filter& filter, int area_level);

}
