#include "compiler/rules.hpp"
#include "compiler/generic.hpp"
#include <vector>

namespace fs::compiler
{

[[nodiscard]]
std::variant<std::vector<lang::filter_block>, error::error_variant> compile_rules(
	const std::vector<parser::ast::action>& top_level_actions,
	const std::vector<parser::ast::rule_block>& top_level_blocks,
	const lang::constants_map& map,
	const parser::lookup_data& lookup_data)
{
	std::variant<lang::action_set, error::error_variant> construction_result = construct_action_set(top_level_actions, map, lookup_data);
	if (std::holds_alternative<error::error_variant>(construction_result))
		return std::get<error::error_variant>(std::move(construction_result));

	std::vector<lang::filter_block> filter_content;
	lang::action_set& action_set = std::get<lang::action_set>(construction_result);
	action_set.show = true; // default action for any top-level block should be show
	for (const parser::ast::rule_block& block : top_level_blocks)
	{
		std::optional<error::error_variant> error =
			// top-level has no conditions so default-construct empty condition set
			add_rules_to_filter(lang::condition_set{}, action_set, block, map, lookup_data, filter_content);

		if (error)
			return *std::move(error);
	}

	// TODO: what to do with the last block?
	// give an option to select: (show/hide/dont generate)
	// now: just continue with 'show' default
	filter_content.push_back(lang::filter_block{lang::condition_set{}, action_set});
	return filter_content;
}

std::optional<error::error_variant> add_rules_to_filter(
	lang::condition_set parent_condition_set,
	lang::action_set parent_action_set,
	const parser::ast::rule_block& block,
	const lang::constants_map& map,
	const parser::lookup_data& lookup_data,
	std::vector<lang::filter_block>& filter_content)
{
//	std::variant<lang::action_set, error::error_variant> action_set_result = construct_action_set(block.actions, map, lookup_data);
//	if (std::holds_alternative<error::error_variant>(action_set_result))
//		return std::get<error::error_variant>(std::move(action_set_result));
//
//	const lang::action_set actions = parent_action_set.override_by(std::get<lang::action_set>(action_set_result));
//
//	std::variant<lang::condition_set, error::error_variant> condition_set_result = construct_condition_set(block.conditions, map, lookup_data);
//	if (std::holds_alternative<error::error_variant>(condition_set_result))
//		return std::get<error::error_variant>(std::move(condition_set_result));
//
//	const lang::condition_set conditions = std::get<lang::condition_set>(parent_condition_set.add_restrictions(std::get<lang::condition_set>(condition_set_result)));
//
//	for (const parser::ast::rule_block& child_block : block.child_blocks)
//	{
//		std::optional<error::error_variant> error =
//			add_rules_to_filter(conditions, actions, child_block, map, lookup_data, filter_content);
//
//		if (error)
//			return *std::move(error);
//	}
//
//	filter_content.push_back(lang::filter_block{lang::condition_set{}, actions});
	assert(false);
	return std::nullopt; // FIXME implement
}

}
