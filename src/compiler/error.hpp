#pragma once
#include "lang/types.hpp"
#include "parser/config.hpp"
#include <variant>

// all possible compilation errors

namespace fs::compiler::error
{

struct no_error {};
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

struct internal_error {};

using error_variant = std::variant<
	no_error,
	name_already_exists,
	no_such_name,
	type_mismatch,
	internal_error
>;

}
