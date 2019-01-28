#pragma once
#include "lang/types.hpp"
#include "parser/config.hpp"
#include <variant>

// all possible compilation errors

namespace fs::compiler::error
{

struct name_already_exists
{
	parser::range_type duplicated_name;
	parser::range_type original_name;
};

struct no_such_name
{
	parser::range_type name_origin;
};

struct type_mismatch
{
	// invariant: left_operand_type != right_operand_type
	lang::object_type left_operand_type;
	lang::object_type right_operand_type;
	parser::range_type right_operand_value_origin;
	// if object was unnamed literal then type origin should point at that literal
	parser::range_type right_operand_type_origin;
};

// inform the user that '[expr]' was probably intended than 'expr'
struct single_object_to_array_assignment
{
	lang::object_type left_operand_type;
	lang::single_object_type right_operand_type;
	parser::range_type right_operand_expr;
};

struct nested_arrays_not_allowed
{
	parser::range_type expression_origin;
};

struct non_homogeneous_array
{
	parser::range_type first_expr_origin;
	parser::range_type second_expr_origin;
	lang::single_object_type first_expr_type;
	lang::single_object_type second_expr_type;
};

struct internal_error_while_parsing_constant
// all available backtrack places when error is generated
{
	parser::range_type wanted_name_origin;
	parser::range_type wanted_type_origin;
	parser::range_type expression_type_origin;
	parser::range_type expression_value_origin;
	std::optional<parser::range_type> expression_name_origin;
};

using error_variant = std::variant<
	name_already_exists,
	no_such_name,
	type_mismatch,
	single_object_to_array_assignment,
	nested_arrays_not_allowed,
	non_homogeneous_array,
	internal_error_while_parsing_constant
>;

}
