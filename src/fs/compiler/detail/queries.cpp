#include "fs/compiler/detail/queries.hpp"
#include "fs/compiler/detail/evaluate_as.hpp"

#include <utility>

namespace fs::compiler::detail
{

std::variant<lang::price_range, error::error_variant> construct_price_range(
	const parser::ast::value_expression_list& arguments,
	const lang::constants_map& map,
	const lang::item_price_data& item_price_data)
{
	const int arguments_amount = arguments.size();
	if (arguments_amount != 2)
		return error::invalid_amount_of_arguments{2, arguments_amount, parser::get_position_info(arguments)};

	std::variant<lang::floating_point, error::error_variant> fp_min_or_error = evaluate_as<lang::floating_point>(arguments[0], map, item_price_data);
	if (std::holds_alternative<error::error_variant>(fp_min_or_error))
		return std::get<error::error_variant>(std::move(fp_min_or_error));

	std::variant<lang::floating_point, error::error_variant> fp_max_or_error = evaluate_as<lang::floating_point>(arguments[1], map, item_price_data);
	if (std::holds_alternative<error::error_variant>(fp_max_or_error))
		return std::get<error::error_variant>(std::move(fp_max_or_error));

	return lang::price_range{
		std::get<lang::floating_point>(fp_min_or_error).value,
		std::get<lang::floating_point>(fp_max_or_error).value};
}

}
