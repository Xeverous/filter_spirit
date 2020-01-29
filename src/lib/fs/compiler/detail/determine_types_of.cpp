#include <fs/compiler/detail/determine_types_of.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/error.hpp>
#include <fs/lang/object.hpp>

namespace fs::compiler::detail
{

std::vector<std::optional<lang::object_type>> determine_types_of(
	const parser::ast::sf::value_expression_list& expressions,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	std::vector<std::optional<lang::object_type>> result;
	result.reserve(expressions.size());

	for (const parser::ast::sf::value_expression& expression : expressions) {
		std::variant<lang::object, compile_error> object_or_error =
			evaluate_value_expression(expression, symbols, item_price_data);

		if (std::holds_alternative<lang::object>(object_or_error)) {
			const auto& object = std::get<lang::object>(object_or_error);
			result.push_back(object.type());
		}
		else {
			result.push_back(std::nullopt);
		}
	}

	return result;
}

}
