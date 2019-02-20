#include "compiler/conditions.hpp"
#include "compiler/evaluate_as.hpp"
#include "compiler/convertions.hpp"
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <utility>

namespace fs::compiler
{

namespace ast = parser::ast;
namespace x3 = boost::spirit::x3;

std::optional<error::error_variant> add_conditions(
	const std::vector<ast::condition>& conditions,
	const lang::constants_map& map,
	lang::condition_set& condition_set)
{
	for (const ast::condition& condition : conditions)
	{
		auto error = condition.apply_visitor(x3::make_lambda_visitor<std::optional<error::error_variant>>(
			[&](const ast::comparison_condition& comparison_condition)
			{
				return add_comparison_condition(comparison_condition, map, condition_set);
			},
			[&](const ast::string_condition& string_condition)
			{
				return add_string_condition(string_condition, map, condition_set);
			},
			[&](const ast::boolean_condition& boolean_condition)
			{
				return add_boolean_condition(boolean_condition, map, condition_set);
			},
			[&](const ast::socket_group_condition& socket_group_condition)
			{
				return add_socket_group_condition(socket_group_condition, map, condition_set);
			}));

		if (error)
			return *error;
	}

	return std::nullopt;
}

std::optional<error::error_variant> add_comparison_condition(
	const ast::comparison_condition& condition,
	const lang::constants_map& map,
	lang::condition_set& condition_set)
{
	switch (condition.property)
	{
		case lang::comparison_condition_property::item_level:
		{
			std::variant<lang::level, error::error_variant> level_or_error = evaluate_as<lang::level>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(level_or_error))
				return std::get<error::error_variant>(level_or_error);

			const auto& level = std::get<lang::level>(level_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, level.value);
			return add_range_condition(nrc, condition_set.item_level);
		}
		case lang::comparison_condition_property::drop_level:
		{
			std::variant<lang::level, error::error_variant> level_or_error = evaluate_as<lang::level>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(level_or_error))
				return std::get<error::error_variant>(level_or_error);

			const auto& level = std::get<lang::level>(level_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, level.value);
			return add_range_condition(nrc, condition_set.drop_level);
		}
		case lang::comparison_condition_property::quality:
		{
			std::variant<lang::quality, error::error_variant> quality_or_error = evaluate_as<lang::quality>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(quality_or_error))
				return std::get<error::error_variant>(quality_or_error);

			const auto& quality = std::get<lang::quality>(quality_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, quality.value);
			return add_range_condition(nrc, condition_set.quality);
		}
		case lang::comparison_condition_property::rarity:
		{
			std::variant<lang::rarity, error::error_variant> rarity_or_error = evaluate_as<lang::rarity>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(rarity_or_error))
				return std::get<error::error_variant>(rarity_or_error);

			const auto& rarity = std::get<lang::rarity>(rarity_or_error);
			lang::rarity_range_condition rrc(condition.comparison_type.value, rarity);
			return add_range_condition(rrc, condition_set.rarity);
		}
		case lang::comparison_condition_property::sockets:
		{
			std::variant<lang::integer, error::error_variant> integer_or_error = evaluate_as<lang::integer>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(integer_or_error))
				return std::get<error::error_variant>(integer_or_error);

			const auto& integer = std::get<lang::integer>(integer_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, integer.value);
			return add_range_condition(nrc, condition_set.sockets);
		}
		case lang::comparison_condition_property::links:
		{
			std::variant<lang::integer, error::error_variant> integer_or_error = evaluate_as<lang::integer>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(integer_or_error))
				return std::get<error::error_variant>(integer_or_error);

			const auto& integer = std::get<lang::integer>(integer_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, integer.value);
			return add_range_condition(nrc, condition_set.links);
		}
		case lang::comparison_condition_property::height:
		{
			std::variant<lang::integer, error::error_variant> integer_or_error = evaluate_as<lang::integer>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(integer_or_error))
				return std::get<error::error_variant>(integer_or_error);

			const auto& integer = std::get<lang::integer>(integer_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, integer.value);
			return add_range_condition(nrc, condition_set.height);
		}
		case lang::comparison_condition_property::width:
		{
			std::variant<lang::integer, error::error_variant> integer_or_error = evaluate_as<lang::integer>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(integer_or_error))
				return std::get<error::error_variant>(integer_or_error);

			const auto& integer = std::get<lang::integer>(integer_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, integer.value);
			return add_range_condition(nrc, condition_set.width);
		}
		case lang::comparison_condition_property::stack_size:
		{
			std::variant<lang::integer, error::error_variant> integer_or_error = evaluate_as<lang::integer>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(integer_or_error))
				return std::get<error::error_variant>(integer_or_error);

			const auto& integer = std::get<lang::integer>(integer_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, integer.value);
			return add_range_condition(nrc, condition_set.stack_size);
		}
		case lang::comparison_condition_property::gem_level:
		{
			std::variant<lang::integer, error::error_variant> integer_or_error = evaluate_as<lang::integer>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(integer_or_error))
				return std::get<error::error_variant>(integer_or_error);

			const auto& integer = std::get<lang::integer>(integer_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, integer.value);
			return add_range_condition(nrc, condition_set.gem_level);
		}
		case lang::comparison_condition_property::map_tier:
		{
			std::variant<lang::integer, error::error_variant> integer_or_error = evaluate_as<lang::integer>(condition.value, map);
			if (std::holds_alternative<error::error_variant>(integer_or_error))
				return std::get<error::error_variant>(integer_or_error);

			const auto& integer = std::get<lang::integer>(integer_or_error);
			lang::numeric_range_condition nrc(condition.comparison_type.value, integer.value);
			return add_range_condition(nrc, condition_set.map_tier);
		}
		default:
		{
			break;
		}

	}

	return error::internal_compiler_error_during_comparison_condition_evaluation{parser::get_position_info(condition)};
}

std::optional<error::error_variant> add_string_condition(
	const parser::ast::string_condition& condition,
	const lang::constants_map& map,
	lang::condition_set& condition_set)
{
	std::variant<lang::array_object, error::error_variant> array_or_error = evaluate_as<lang::array_object>(condition.value, map);
	if (std::holds_alternative<error::error_variant>(array_or_error))
		return std::get<error::error_variant>(array_or_error);

	auto& array = std::get<lang::array_object>(array_or_error);
	std::variant<std::vector<std::string>, error::error_variant> strings_or_error = array_to_strings(std::move(array));
	if (std::holds_alternative<error::error_variant>(strings_or_error))
		return std::get<error::error_variant>(strings_or_error);

	auto& strings = std::get<std::vector<std::string>>(strings_or_error);
	lang::position_tag condition_origin = parser::get_position_info(condition);

	switch (condition.property)
	{
		case lang::string_condition_property::class_:
		{
			return add_strings_condition(std::move(strings), condition_origin, condition_set.class_);
		}
		case lang::string_condition_property::base_type:
		{
			return add_strings_condition(std::move(strings), condition_origin, condition_set.base_type);
		}
		case lang::string_condition_property::has_explicit_mod:
		{
			return add_strings_condition(std::move(strings), condition_origin, condition_set.has_explicit_mod);
		}
		default:
		{
			break;
		}
	}

	return error::internal_compiler_error_during_string_condition_evaluation{condition_origin};
}

std::optional<error::error_variant> add_strings_condition(
	std::vector<std::string> strings,
	lang::position_tag condition_origin,
	std::shared_ptr<std::vector<std::string>>& target)
{
	if (target != nullptr)
	{
		// error situation: parent block has already set a condition
		// conditions do not override, only inherit
		return error::condition_redefinition{condition_origin};
	}

	target = std::make_shared<std::vector<std::string>>(std::move(strings));
	return std::nullopt;
}

std::optional<error::error_variant> add_boolean_condition(
	const parser::ast::boolean_condition& condition,
	const lang::constants_map& map,
	lang::condition_set& condition_set)
{
	std::variant<lang::boolean, error::error_variant> boolean_or_error = evaluate_as<lang::boolean>(condition.value, map);
	if (std::holds_alternative<error::error_variant>(boolean_or_error))
			return std::get<error::error_variant>(boolean_or_error);

	auto& boolean = std::get<lang::boolean>(boolean_or_error);
	lang::position_tag condition_origin = parser::get_position_info(condition);

	switch (condition.property)
	{
		case lang::boolean_condition_property::identified:
		{
			return add_boolean_condition(boolean, condition_origin, condition_set.is_identified);
		}
		case lang::boolean_condition_property::corrupted:
		{
			return add_boolean_condition(boolean, condition_origin, condition_set.is_corrupted);
		}
		case lang::boolean_condition_property::elder_item:
		{
			return add_boolean_condition(boolean, condition_origin, condition_set.is_elder_item);
		}
		case lang::boolean_condition_property::shaper_item:
		{
			return add_boolean_condition(boolean, condition_origin, condition_set.is_shaper_item);
		}
		case lang::boolean_condition_property::shaped_map:
		{
			return add_boolean_condition(boolean, condition_origin, condition_set.is_shaped_map);
		}
		default:
		{
			break;
		}
	}

	return error::internal_compiler_error_during_boolean_condition_evaluation{condition_origin};
}

std::optional<error::error_variant> add_boolean_condition(
	lang::boolean boolean,
	lang::position_tag condition_origin,
	std::optional<lang::boolean>& target)
{
	if (target.has_value())
		return error::condition_redefinition{condition_origin};

	target = boolean;
	return std::nullopt;
}

std::optional<error::error_variant> add_socket_group_condition(
	const parser::ast::socket_group_condition& condition,
	const lang::constants_map& map,
	lang::condition_set& condition_set)
{
	std::variant<lang::socket_group, error::error_variant> socket_group_or_error = evaluate_as<lang::socket_group>(condition.value, map);
	if (std::holds_alternative<error::error_variant>(socket_group_or_error))
			return std::get<error::error_variant>(socket_group_or_error);

	const auto& socket_group = std::get<lang::socket_group>(socket_group_or_error);
	lang::position_tag condition_origin = parser::get_position_info(condition);
	return add_socket_group_condition(socket_group, condition_origin, condition_set.socket_group);
}

std::optional<error::error_variant> add_socket_group_condition(
	lang::socket_group socket_group,
	lang::position_tag condition_origin,
	std::optional<lang::socket_group>& target)
{
	if (target.has_value())
		return error::condition_redefinition{condition_origin};

	target = socket_group;
	return std::nullopt;
}

}
