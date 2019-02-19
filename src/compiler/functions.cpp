#include "compiler/functions.hpp"
#include "compiler/evaluate.hpp"
#include "compiler/evaluate_as.hpp"
#include <array>
#include <cassert>

namespace fs::compiler
{

std::variant<lang::color, error::error_variant> construct_color(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map)
{
	int arguments_amount = arguments.size();
	if (arguments_amount != 3 && arguments_amount != 4)
		return error::invalid_amount_of_arguments{3, 4, parser::get_position_info(arguments)};

	std::array<int, 3> rgb_values;
	for (std::size_t i = 0; i < rgb_values.size(); ++i)
	{
		std::variant<lang::integer, error::error_variant> int_or_error = evaluate_as<lang::integer>(arguments[i], map);
		if (std::holds_alternative<error::error_variant>(int_or_error))
			return std::get<error::error_variant>(int_or_error);

		auto& integer = std::get<lang::integer>(int_or_error);
		rgb_values[i] = integer.value;
	}

	if (arguments_amount == 3)
		return lang::color(rgb_values[0], rgb_values[1], rgb_values[2]);

	assert(arguments_amount == 4);
	std::variant<lang::integer, error::error_variant> int_or_error = evaluate_as<lang::integer>(arguments[3], map);
	if (std::holds_alternative<error::error_variant>(int_or_error))
		return std::get<error::error_variant>(int_or_error);

	auto& integer = std::get<lang::integer>(int_or_error);
	return lang::color(rgb_values[0], rgb_values[1], rgb_values[2], integer.value);
}

}
