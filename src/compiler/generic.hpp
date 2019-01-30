#pragma once
#include "compiler/compiler.hpp"
#include "compiler/process_input.hpp"
#include "compiler/error.hpp"
#include "lang/types.hpp"
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
lang::single_object_type type_of_single_object(const lang::single_object& obj);

[[nodiscard]]
lang::object_type type_of_object(const lang::object& obj);

[[nodiscard]]
lang::single_object literal_to_single_object(const parser::ast::literal_expression& literal);

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

[[nodiscard]]
std::variant<lang::object, error::error_variant> construct_object_of_type(
	lang::object_type wanted_type,
	lang::object&& object);

}
