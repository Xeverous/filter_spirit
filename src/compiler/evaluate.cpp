#include "compiler/evaluate.hpp"
#include "compiler/functions.hpp"
#include "compiler/queries.hpp"
#include "lang/functions.hpp"
#include "lang/queries.hpp"
#include <utility>
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

namespace fs::compiler
{

namespace x3 = boost::spirit::x3;
namespace ast = parser::ast;

std::variant<lang::object, error::error_variant> evaluate_value_expression(
	const ast::value_expression& value_expression,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	using result_type = std::variant<lang::object, error::error_variant>;

	return value_expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[](const ast::literal_expression& literal) -> result_type {
			return evaluate_literal(literal);
		},
		[&](const ast::array_expression& array) {
			return evaluate_array(array, map);
		},
		[&](const ast::function_call& function_call) {
			return evaluate_function_call(function_call, map);
		},
		[&](const ast::price_range_query& price_range_query) {
			return evaluate_price_range_query(price_range_query, map, item_price_data);
		},
		[&](const ast::identifier& identifier) {
			return evaluate_identifier(identifier, map);
		}
	));
}

lang::object evaluate_literal(
	const ast::literal_expression& expression)
{
	using result_type = lang::single_object;

	result_type object = expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[](ast::boolean_literal literal) -> result_type {
			return lang::boolean{literal.value};
		},
		[](ast::integer_literal literal) -> result_type {
			return lang::integer{literal.value};
		},
		[](ast::rarity_literal literal) -> result_type {
			return lang::rarity{literal.value};
		},
		[](ast::shape_literal literal) -> result_type {
			return lang::shape{literal.value};
		},
		[](ast::suit_literal literal) -> result_type {
			return lang::suit{literal.value};
		},
		[](const ast::string_literal& literal) -> result_type {
			return lang::string{literal};
		}
	));

	return lang::object{
		std::move(object),
		parser::get_position_info(expression),
		std::nullopt};
}

std::variant<lang::object, error::error_variant> evaluate_array(
	const ast::array_expression& expression,
	const lang::constants_map& map)
{
	// note: the entire function should work also in case of empty array
	lang::array_object array;
	for (const ast::value_expression& value_expression : expression)
	{
		std::variant<lang::object, error::error_variant> object_or_error = evaluate_value_expression(value_expression, map);
		if (std::holds_alternative<error::error_variant>(object_or_error))
			return std::get<error::error_variant>(object_or_error);

		auto& object = std::get<lang::object>(object_or_error);

		if (object.is_array())
			return error::nested_arrays_not_allowed{parser::get_position_info(value_expression)};

		array.push_back(std::move(object));
	}

	std::optional<error::non_homogeneous_array> homogenity_error = verify_array_homogeneity(array);
	if (homogenity_error)
		return *homogenity_error;

	return lang::object{
		std::move(array),
		parser::get_position_info(expression),
		std::nullopt};
}

std::variant<lang::object, error::error_variant> evaluate_function_call(
	const ast::function_call& function_call,
	const lang::constants_map& map)
{
	/*
	 * right now there is no support for user-defined functions
	 * so just compare function name against built-in functions
	 *
	 * if there is a need to support user-defined functions,
	 * they can be stored in the map
	 */
	const ast::identifier& function_name = function_call.name;
	const ast::function_arguments& arguments = function_call.arguments;
	/*
	 * note: this is O(n) but relying on small string optimization
	 * and much better memory layout makes it to run faster than a
	 * tree-based or hash-based map. We can also optimize order of
	 * comparisons.
	 */
	if (function_name.value == lang::functions::rgb)
	{
		std::variant<lang::color, error::error_variant> color_or_error = construct_color(arguments, map);
		if (std::holds_alternative<error::error_variant>(color_or_error))
			return std::get<error::error_variant>(color_or_error);

		return lang::object{
			lang::single_object(std::get<lang::color>(color_or_error)),
			parser::get_position_info(function_call),
			std::nullopt};
	}
	else if (function_name.value == lang::functions::group)
	{
		std::variant<lang::socket_group, error::error_variant> socket_group_or_error = construct_socket_group(arguments, map);
		if (std::holds_alternative<error::error_variant>(socket_group_or_error))
			return std::get<error::error_variant>(socket_group_or_error);

		return lang::object{
			lang::single_object(std::get<lang::socket_group>(socket_group_or_error)),
			parser::get_position_info(function_call),
			std::nullopt};
	}
	else if (function_name.value == lang::functions::minimap_icon)
	{
		std::variant<lang::minimap_icon, error::error_variant> icon_or_error = construct_minimap_icon(arguments, map);
		if (std::holds_alternative<error::error_variant>(icon_or_error))
			return std::get<error::error_variant>(icon_or_error);

		return lang::object{
			lang::single_object(std::get<lang::minimap_icon>(icon_or_error)),
			parser::get_position_info(function_call),
			std::nullopt};
	}
	else if (function_name.value == lang::functions::beam_effect)
	{
		std::variant<lang::beam_effect, error::error_variant> beam_effect_or_error = construct_beam_effect(arguments, map);
		if (std::holds_alternative<error::error_variant>(beam_effect_or_error))
			return std::get<error::error_variant>(beam_effect_or_error);

		return lang::object{
			lang::single_object(std::get<lang::beam_effect>(beam_effect_or_error)),
			parser::get_position_info(function_call),
			std::nullopt};
	}
	else if (function_name.value == lang::functions::path)
	{
		std::variant<lang::path, error::error_variant> path_or_error = construct_path(arguments, map);
		if (std::holds_alternative<error::error_variant>(path_or_error))
			return std::get<error::error_variant>(path_or_error);

		return lang::object{
			lang::single_object(std::get<lang::path>(path_or_error)),
			parser::get_position_info(function_call),
			std::nullopt};
	}

	return error::no_such_function{parser::get_position_info(function_name)};
}


std::variant<lang::object, error::error_variant> evaluate_price_range_query(
	const parser::ast::price_range_query& price_range_query,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	std::variant<lang::price_range, error::error_variant> range_or_error = construct_price_range(price_range_query.arguments, map);
	if (std::holds_alternative<error::error_variant>(range_or_error))
		return std::get<error::error_variant>(range_or_error);

	const auto& price_range = std::get<lang::price_range>(range_or_error);

	/*
	 * note: this is O(n) but relying on small string optimization
	 * and much better memory layout makes it to run faster than a
	 * tree-based or hash-based map. We can also optimize order of
	 * comparisons.
	 */
	const ast::identifier& query_name = price_range_query.name;
	if (query_name.value == lang::queries::divination)
	{
		const std::vector<itemdata::divination_card>& cards = item_price_data.divination_cards;
		return evaluate_price_range_query_on_sorted_range(price_range, cards.begin(), cards.end());
	}
	else if (query_name.value == lang::queries::prophecies)
	{
		const std::vector<itemdata::prophecy>& prophecies = item_price_data.prophecies;
		return evaluate_price_range_query_on_sorted_range(price_range, prophecies.begin(), prophecies.end());
	}

	return error::no_such_query{parser::get_position_info(query_name)};
}

std::variant<lang::object, error::error_variant> evaluate_identifier(
	const ast::identifier& identifier,
	const lang::constants_map& map)
{
	const lang::position_tag place_of_name = parser::get_position_info(identifier);

	const auto it = map.find(identifier.value); // C++17: use if (expr; cond)
	if (it == map.end())
		return error::no_such_name{place_of_name};

	return lang::object{
		it->second.value,
		place_of_name,
		std::nullopt
	};
}

std::optional<error::non_homogeneous_array> verify_array_homogeneity(
	const lang::array_object& array)
{
	if (array.empty())
		return std::nullopt;

	const lang::object& first_object = array.front();
	const lang::object_type first_type = lang::type_of_object(first_object);
	for (const lang::object& element : array)
	{
		const lang::object_type tested_type = lang::type_of_object(element);
		if (tested_type != first_type) // C++20: [[unlikely]]
		{
			return error::non_homogeneous_array{
				first_object.value_origin,
				element.value_origin,
				first_type,
				tested_type};
		}
	}

	return std::nullopt;
}

}
