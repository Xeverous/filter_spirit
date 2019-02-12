#pragma once
#include "compiler/compiler.hpp"
#include "compiler/process_input.hpp"
#include "compiler/error.hpp"
#include "lang/types.hpp"
#include "lang/condition_set.hpp"
#include "lang/action_set.hpp"
#include "parser/ast.hpp"
#include "parser/parser.hpp"
#include <string_view>
#include <optional>

namespace fs::compiler
{

/**
 * @brief attempt to convert sequence of characters (eg RRGBW) to a group
 * @return empty if string was not valid
 */
[[nodiscard]]
std::optional<lang::group> identifier_to_group(std::string_view identifier);

[[nodiscard]]
lang::color color_literal_to_color(
	parser::ast::color_literal lit,
	const parser::lookup_data& lookup_data);

[[nodiscard]]
lang::single_object literal_to_single_object(
	const parser::ast::literal_expression& literal,
	const parser::lookup_data& lookup_data);

[[nodiscard]]
lang::object_type type_expression_to_type(const parser::ast::type_expression& type_expr);

// array MUST NOT be empty
// both containers MUST BE of the same size
[[nodiscard]]
std::optional<error::non_homogeneous_array> verify_homogeneity(
	const lang::array_object& array,
	const std::vector<parser::range_type>& origins);

[[nodiscard]]
std::variant<lang::object, error::error_variant> identifier_to_object(
	const parser::ast::identifier& identifier,
	parser::range_type position_of_identifier,
	const constants_map& map);

[[nodiscard]]
std::variant<lang::object, error::error_variant> expression_to_object(
	const parser::ast::value_expression& value_expression,
	const parser::lookup_data& lookup_data,
	const constants_map& map);

[[nodiscard]]
std::variant<lang::single_object, error::error_variant> construct_single_object_of_type(
	lang::single_object_type wanted_type,
	lang::single_object object,
	parser::range_type object_value_origin,
	parser::range_type object_type_origin);

[[nodiscard]]
std::variant<lang::single_object, error::error_variant> construct_single_object_of_type(
	lang::single_object_type wanted_type,
	lang::object object);

[[nodiscard]]
std::variant<lang::array_object, error::error_variant> construct_array_object_of_type(
	lang::single_object_type inner_array_type,
	lang::object object);

[[nodiscard]] // FIXME: does not set returned object's origins
std::variant<lang::object, error::error_variant> construct_object_of_type(
	lang::object_type wanted_type,
	lang::object&& object);

[[nodiscard]]
std::variant<lang::color, error::error_variant> color_expression_to_color(
	const parser::ast::color_expression& expr,
	const constants_map& map,
	const parser::lookup_data& lookup_data);

[[nodiscard]]
std::variant<lang::level, error::error_variant> level_expression_to_level(
	const parser::ast::level_expression& expr,
	const constants_map& map,
	const parser::lookup_data& lookup_data);

[[nodiscard]]
std::variant<lang::action_set, error::error_variant> construct_action_set(
	const parser::ast::action_list& action_list,
	const constants_map& map,
	const parser::lookup_data& lookup_data);

[[nodiscard]]
std::variant<lang::condition_set, error::error_variant> construct_condition_set(
	const parser::ast::condition_list& condition_list,
	const constants_map& map,
	const parser::lookup_data& lookup_data);


// FIXME: investigate code duplication and potential missing promotion bugs with:
// - identifier_to_object
// - construct_{single|array|}object_of_type
template <typename T> [[nodiscard]]
std::variant<T, error::error_variant> identifier_to_type(
	const parser::ast::identifier& identifier,
	const constants_map& map,
	const parser::lookup_data& lookup_data)
{
	const auto it = map.find(identifier.value);
	if (it == map.end())
	{
		return error::no_such_name{
			lookup_data.position_of(identifier)
		};
	}

	const lang::object& object = it->second;

	if (std::holds_alternative<lang::array_object>(object.value))
	{
		return error::type_mismatch_in_expression{
			lang::type_to_enum<T>(),
			lang::type_of_object(object),
			object.type_origin
		};
	}

	const auto& single_object = std::get<lang::single_object>(object.value);

	if (!std::holds_alternative<T>(single_object))
	{
		return error::type_mismatch_in_expression{
			lang::type_to_enum<T>(),
			lang::type_of_object(object),
			object.type_origin
		};
	}

	return std::get<T>(single_object);
}

}
