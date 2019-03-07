#include "compiler/queries.hpp"
#include "compiler/evaluate_as.hpp"

namespace fs::compiler
{

std::variant<lang::price_range, error::error_variant> construct_price_range(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map)
{
	const int arguments_amount = arguments.size();
	if (arguments_amount != 2)
		return error::invalid_amount_of_arguments{2, 2, arguments_amount, parser::get_position_info(arguments)};

	std::variant<lang::floating_point, error::error_variant> fp_min_or_error = evaluate_as<lang::floating_point>(arguments[0], map);
	if (std::holds_alternative<error::error_variant>(fp_min_or_error))
		return std::get<error::error_variant>(fp_min_or_error);

	std::variant<lang::floating_point, error::error_variant> fp_max_or_error = evaluate_as<lang::floating_point>(arguments[1], map);
	if (std::holds_alternative<error::error_variant>(fp_max_or_error))
		return std::get<error::error_variant>(fp_max_or_error);

	return lang::price_range{
		std::get<lang::floating_point>(fp_min_or_error).value,
		std::get<lang::floating_point>(fp_max_or_error).value};
}

}
