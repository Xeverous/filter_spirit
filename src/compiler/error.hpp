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
	internal_error_while_parsing_constant
>;

}
