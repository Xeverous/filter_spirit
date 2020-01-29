#include <fs/compiler/detail/queries.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/error.hpp>
#include <fs/lang/price_range.hpp>
#include <fs/lang/object.hpp>

#include <utility>

namespace
{

using namespace fs;
using fs::compiler::compile_error;

[[nodiscard]] std::variant<std::optional<double>, compile_error>
evaluate_range_value(const lang::object& obj)
{
	lang::object_type type = obj.type();
	if (type == +lang::object_type::integer)
		return std::get<lang::integer>(obj.value).value;
	if (type == +lang::object_type::floating_point)
		return std::get<lang::floating_point>(obj.value).value;
	if (type == +lang::object_type::none)
		return std::nullopt;

	// TODO this needs better error handling - fix this when price query design for non-elementary items
	// is completed (and likely remove item_price_data from most of the code)
	return compiler::errors::type_mismatch{
		lang::object_type::floating_point,
		type,
		obj.value_origin
	};
}

}

namespace fs::compiler::detail
{

std::variant<lang::price_range, compile_error>
construct_price_range(
	const parser::ast::sf::value_expression_list& arguments,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	const int arguments_amount = arguments.size();
	if (arguments_amount != 2)
		return errors::invalid_amount_of_arguments{2, arguments_amount, parser::get_position_info(arguments)};

	std::variant<lang::object, compile_error> min_or_error = evaluate_value_expression(arguments[0], symbols, item_price_data);
	if (std::holds_alternative<compile_error>(min_or_error))
		return std::get<compile_error>(std::move(min_or_error));

	const auto& min_obj = std::get<lang::object>(min_or_error);
	std::variant<std::optional<double>, compile_error> min_val_or_error = evaluate_range_value(min_obj);
	if (std::holds_alternative<compile_error>(min_val_or_error))
		return std::get<compile_error>(std::move(min_val_or_error));

	std::variant<lang::object, compile_error> max_or_error = evaluate_value_expression(arguments[1], symbols, item_price_data);
	if (std::holds_alternative<compile_error>(max_or_error))
		return std::get<compile_error>(std::move(max_or_error));

	const auto& max_obj = std::get<lang::object>(max_or_error);
	std::variant<std::optional<double>, compile_error> max_val_or_error = evaluate_range_value(max_obj);
	if (std::holds_alternative<compile_error>(max_val_or_error))
		return std::get<compile_error>(std::move(max_val_or_error));

	return lang::price_range{
		std::get<std::optional<double>>(min_val_or_error),
		std::get<std::optional<double>>(max_val_or_error)
	};
}

}
