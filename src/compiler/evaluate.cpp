#include "compiler/evaluate.hpp"
#include <utility>
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

namespace fs::compiler
{

namespace x3 = boost::spirit::x3;
namespace ast = parser::ast;

std::variant<lang::object, error::error_variant> evaluate_value_expression(
	const ast::value_expression& value_expression,
	const lang::constants_map& map)
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
			return lang::number{literal.value};
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
		std::nullopt
	};
}

std::variant<lang::object, error::error_variant> evaluate_function_call(
	const ast::function_call& function_call,
	const lang::constants_map& map)
{
	/*
	 * right now there is no support for user-defined functions
	 * so just compare function name against built-in functions
	 */
	const ast::identifier& name = function_call.name;
	// TODO functions
	return error::no_such_function{parser::get_position_info(name)};
}

std::variant<lang::object, error::error_variant> evaluate_identifier(
	const ast::identifier& identifier,
	const lang::constants_map& map)
{
	const auto it = map.find(identifier.value); // C++17: use if (expr; cond)
	const lang::position_tag place_of_name = parser::get_position_info(identifier);

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
