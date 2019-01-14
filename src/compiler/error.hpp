#pragma once
#include <variant>

// all possible compilation errors

namespace fs::compiler
{

struct no_error {};
struct name_already_exists {};
struct no_such_name {};
struct type_mismatch {};


using error_type = std::variant<
	no_error,
	name_already_exists,
	no_such_name,
	type_mismatch
>;

}
