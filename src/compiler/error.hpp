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
	//parser::range_type place_of_error;
	lang::object_type expected;
	lang::object_type actual;
	parser::range_type type_origin;
	parser::range_type value_origin;
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
