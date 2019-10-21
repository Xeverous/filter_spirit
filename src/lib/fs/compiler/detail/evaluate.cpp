#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/evaluate_as.hpp>
#include <fs/compiler/detail/get_value_as.hpp>
#include <fs/compiler/detail/queries.hpp>
#include <fs/compiler/detail/construct.hpp>
#include <fs/compiler/detail/add_action.hpp>
#include <fs/lang/functions.hpp>
#include <fs/lang/queries.hpp>
#include <fs/lang/price_range.hpp>

#include <cassert>
#include <utility>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

namespace x3 = boost::spirit::x3;

using namespace fs;
using namespace fs::compiler;
namespace ast = fs::parser::ast;

namespace
{

[[nodiscard]] lang::object
evaluate_literal(
	const ast::literal_expression& expression)
{
	using result_type = lang::object_variant;

	auto object = expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[](ast::boolean_literal literal) -> result_type {
			return lang::boolean{literal.value};
		},
		[](ast::integer_literal literal) -> result_type {
			return lang::integer{literal.value};
		},
		[](ast::floating_point_literal literal) -> result_type {
			return lang::floating_point{literal.value};
		},
		[](ast::rarity_literal literal) -> result_type {
			return lang::rarity{literal.value};
		},
		[](ast::shape_literal literal) -> result_type {
			return lang::shape{literal.value};
		},
		[](ast::suit_literal literal) -> result_type {
			return lang::suit{literal.value};
		},
		[](const ast::string_literal& literal) -> result_type {
			return lang::string{literal};
		}
	));

	return lang::object{std::move(object), parser::get_position_info(expression)};
}

[[nodiscard]] std::optional<errors::non_homogeneous_array>
verify_array_homogeneity(
	const lang::array_object& array)
{
	if (array.empty())
		return std::nullopt;

	const lang::object& first_object = array.front();
	const lang::object_type first_type = lang::type_of_object(first_object.value);
	for (const lang::object& element : array)
	{
		const lang::object_type tested_type = lang::type_of_object(element.value);
		if (tested_type != first_type) // C++20: [[unlikely]]
		{
			return errors::non_homogeneous_array{
				first_object.value_origin,
				element.value_origin,
				first_type,
				tested_type};
		}
	}

	return std::nullopt;
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_array(
	const ast::array_expression& expression,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	// note: the entire function should work also in case of empty array
	lang::array_object array;
	for (const ast::value_expression& value_expression : expression.elements)
	{
		std::variant<lang::object, compile_error> object_or_error =
			compiler::detail::evaluate_value_expression(value_expression, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(object_or_error))
			return std::get<compile_error>(std::move(object_or_error));

		auto& object = std::get<lang::object>(object_or_error);

		if (object.is_array())
			return errors::nested_arrays_not_allowed{parser::get_position_info(value_expression)};

		array.push_back(std::move(object));
	}

	std::optional<errors::non_homogeneous_array> homogenity_error = verify_array_homogeneity(array);
	if (homogenity_error)
		return *homogenity_error;

	return lang::object{
		std::move(array),
		parser::get_position_info(expression)};
}

/*
 * convert language's subscript index to actual (C++) array index
 * negative index is nth element from the end
 *
 * example:
 *
 * array_size = 4
 * array[0] => 0
 * array[3] => 3
 * array[4] => error (empty result)
 * array[-1] => 3
 * array[-4] => 0
 * array[-5] => error (empty result)
 */
[[nodiscard]] std::optional<int>
subscript_index_to_array_index(int n, int array_size)
{
	assert(array_size >= 0);

	if (n >= 0)
	{
		if (n < array_size)
			return n;
		else
			return std::nullopt;
	}
	else
	{
		if (n + array_size >= 0)
			return n + array_size;
		else
			return std::nullopt;
	}
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_subscript(
	const lang::object& obj,
	const ast::subscript& subscript,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	std::variant<lang::array_object, compile_error> array_or_error =
		fs::compiler::detail::get_value_as<lang::array_object, false>(obj);
	if (std::holds_alternative<compile_error>(array_or_error))
		return std::get<compile_error>(std::move(array_or_error));

	std::variant<lang::integer, compile_error> subscript_or_error =
		fs::compiler::detail::evaluate_as<lang::integer, false>(subscript.expr, symbols, item_price_data);
	if (std::holds_alternative<compile_error>(subscript_or_error))
		return std::get<compile_error>(std::move(subscript_or_error));

	auto& array = std::get<lang::array_object>(array_or_error);
	auto& subscript_index = std::get<lang::integer>(subscript_or_error);

	const int array_size = array.size();
	std::optional<int> index = subscript_index_to_array_index(subscript_index.value, array_size);
	if (!index)
	{
		return errors::index_out_of_range{parser::get_position_info(subscript), subscript_index.value, array_size};
	}

	return lang::object{std::move(array[*index].value), parser::get_position_info(subscript)};
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_function_call(
	const ast::function_call& function_call,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	/*
	 * right now there is no support for user-defined functions
	 * so just compare function name against built-in functions
	 *
	 * if there is a need to support user-defined functions,
	 * they can be stored in the symbol_table
	 */
	const ast::identifier& function_name = function_call.name;
	/*
	 * note: this is O(n) but relying on small string optimization
	 * and much better memory layout makes it to run faster than a
	 * tree-based or hash-based map. We can also optimize order of
	 * comparisons.
	 */
	using compiler::detail::construct;
	if (function_name.value == lang::functions::rgb)
	{
		std::variant<lang::color, compile_error> color_or_error = construct<lang::color>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(color_or_error))
			return std::get<compile_error>(std::move(color_or_error));

		return lang::object{
			std::get<lang::color>(color_or_error),
			parser::get_position_info(function_call)};
	}
	else if (function_name.value == lang::functions::level)
	{
		std::variant<lang::level, compile_error> level_or_error = construct<lang::level>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(level_or_error))
			return std::get<compile_error>(std::move(level_or_error));

		return lang::object{
			std::get<lang::level>(level_or_error),
			parser::get_position_info(function_call)};
	}
	else if (function_name.value == lang::functions::font_size)
	{
		std::variant<lang::font_size, compile_error> font_size_or_error = construct<lang::font_size>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(font_size_or_error))
			return std::get<compile_error>(std::move(font_size_or_error));

		return lang::object{
			std::get<lang::font_size>(font_size_or_error),
			parser::get_position_info(function_call)};
	}
	else if (function_name.value == lang::functions::sound_id)
	{
		std::variant<lang::sound_id, compile_error> sound_id_or_error = construct<lang::sound_id>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(sound_id_or_error))
			return std::get<compile_error>(std::move(sound_id_or_error));

		return lang::object{
			std::get<lang::sound_id>(sound_id_or_error),
			parser::get_position_info(function_call)};
	}
	else if (function_name.value == lang::functions::volume)
	{
		std::variant<lang::volume, compile_error> volume_or_error = construct<lang::volume>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(volume_or_error))
			return std::get<compile_error>(std::move(volume_or_error));

		return lang::object{
			std::get<lang::volume>(volume_or_error),
			parser::get_position_info(function_call)};
	}
	else if (function_name.value == lang::functions::group)
	{
		std::variant<lang::socket_group, compile_error> socket_group_or_error = construct<lang::socket_group>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(socket_group_or_error))
			return std::get<compile_error>(std::move(socket_group_or_error));

		return lang::object{
			std::get<lang::socket_group>(socket_group_or_error),
			parser::get_position_info(function_call)};
	}
	else if (function_name.value == lang::functions::minimap_icon)
	{
		std::variant<lang::minimap_icon, compile_error> icon_or_error = construct<lang::minimap_icon>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(icon_or_error))
			return std::get<compile_error>(std::move(icon_or_error));

		return lang::object{
			std::get<lang::minimap_icon>(icon_or_error),
			parser::get_position_info(function_call)};
	}
	else if (function_name.value == lang::functions::beam_effect)
	{
		std::variant<lang::beam_effect, compile_error> beam_effect_or_error = construct<lang::beam_effect>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(beam_effect_or_error))
			return std::get<compile_error>(std::move(beam_effect_or_error));

		return lang::object{
			std::get<lang::beam_effect>(beam_effect_or_error),
			parser::get_position_info(function_call)};
	}
	else if (function_name.value == lang::functions::path)
	{
		std::variant<lang::path, compile_error> path_or_error = construct<lang::path>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(path_or_error))
			return std::get<compile_error>(std::move(path_or_error));

		return lang::object{
			std::get<lang::path>(path_or_error),
			parser::get_position_info(function_call)};
	}
	else if (function_name.value == lang::functions::alert_sound)
	{
		std::variant<lang::alert_sound, compile_error> alert_sound_or_error = construct<lang::alert_sound>(function_call, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(alert_sound_or_error))
			return std::get<compile_error>(std::move(alert_sound_or_error));

		return lang::object{
			std::get<lang::alert_sound>(alert_sound_or_error),
			parser::get_position_info(function_call)};
	}

	return errors::no_such_function{parser::get_position_info(function_name)};
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_price_range_query(
	const ast::price_range_query& price_range_query,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	std::variant<lang::price_range, compile_error> range_or_error =
		compiler::detail::construct_price_range(price_range_query.arguments, symbols, item_price_data);
	if (std::holds_alternative<compile_error>(range_or_error))
		return std::get<compile_error>(std::move(range_or_error));

	const auto& price_range = std::get<lang::price_range>(range_or_error);
	const lang::position_tag position_of_query = parser::get_position_info(price_range_query);

	const auto eval_query = [&](const auto& items)
	{
		lang::array_object array = compiler::detail::evaluate_price_range_query_on_sorted_range(
			price_range,
			position_of_query,
			items.begin(),
			items.end());
		return lang::object{std::move(array), position_of_query};
	};
	/*
	 * note: this is O(n) but relying on small string optimization
	 * and much better memory layout makes it to run faster than a
	 * tree-based or hash-based map. We can also optimize order of
	 * comparisons.
	 */
	const ast::identifier& query_name = price_range_query.name;
	if (query_name.value == lang::queries::divination)
	{
		return eval_query(item_price_data.divination_cards);
	}
	else if (query_name.value == lang::queries::prophecies)
	{
		return eval_query(item_price_data.prophecies);
	}
	else if (query_name.value == lang::queries::essences)
	{
		return eval_query(item_price_data.essences);
	}
	else if (query_name.value == lang::queries::leaguestones)
	{
		return eval_query(item_price_data.leaguestones);
	}
	else if (query_name.value == lang::queries::pieces)
	{
		return eval_query(item_price_data.pieces);
	}
	else if (query_name.value == lang::queries::nets)
	{
		return eval_query(item_price_data.nets);
	}
	else if (query_name.value == lang::queries::vials)
	{
		return eval_query(item_price_data.vials);
	}
	else if (query_name.value == lang::queries::fossils)
	{
		return eval_query(item_price_data.fossils);
	}
	else if (query_name.value == lang::queries::resonators)
	{
		return eval_query(item_price_data.resonators);
	}
	else if (query_name.value == lang::queries::scarabs)
	{
		return eval_query(item_price_data.scarabs);
	}
	else if (query_name.value == lang::queries::uniques_ambiguous_armour)
	{
		return eval_query(item_price_data.ambiguous_unique_armours);
	}
	else if (query_name.value == lang::queries::uniques_ambiguous_weapon)
	{
		return eval_query(item_price_data.ambiguous_unique_weapons);
	}
	else if (query_name.value == lang::queries::uniques_ambiguous_accessory)
	{
		return eval_query(item_price_data.ambiguous_unique_accessories);
	}
	else if (query_name.value == lang::queries::uniques_ambiguous_jewel)
	{
		return eval_query(item_price_data.ambiguous_unique_jewels);
	}
	else if (query_name.value == lang::queries::uniques_ambiguous_flask)
	{
		return eval_query(item_price_data.ambiguous_unique_flasks);
	}
	else if (query_name.value == lang::queries::uniques_ambiguous_map)
	{
		return eval_query(item_price_data.ambiguous_unique_maps);
	}
	else if (query_name.value == lang::queries::uniques_unambiguous_armour)
	{
		return eval_query(item_price_data.unambiguous_unique_armours);
	}
	else if (query_name.value == lang::queries::uniques_unambiguous_weapon)
	{
		return eval_query(item_price_data.unambiguous_unique_weapons);
	}
	else if (query_name.value == lang::queries::uniques_unambiguous_accessory)
	{
		return eval_query(item_price_data.unambiguous_unique_accessories);
	}
	else if (query_name.value == lang::queries::uniques_unambiguous_jewel)
	{
		return eval_query(item_price_data.unambiguous_unique_jewels);
	}
	else if (query_name.value == lang::queries::uniques_unambiguous_flask)
	{
		return eval_query(item_price_data.unambiguous_unique_flasks);
	}
	else if (query_name.value == lang::queries::uniques_unambiguous_map)
	{
		return eval_query(item_price_data.unambiguous_unique_maps);
	}

	return errors::no_such_query{parser::get_position_info(query_name)};
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_identifier(
	const ast::identifier& identifier,
	const lang::symbol_table& symbols)
{
	const lang::position_tag place_of_name = parser::get_position_info(identifier);

	const auto it = symbols.find(identifier.value);
	if (it == symbols.end())
		return errors::no_such_name{place_of_name};

	return lang::object{it->second.object_instance.value, place_of_name};
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_compound_action(
	const ast::compound_action_expression& expr,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	lang::action_set actions;

	for (const ast::action& act : expr) {
		std::optional<compile_error> error = detail::add_action(act, symbols, item_price_data, actions);

		if (error)
			return *std::move(error);
	}

	return lang::object{std::move(actions), parser::get_position_info(expr)};
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_primary_expression(
	const ast::primary_expression& primary_expression,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	using result_type = std::variant<lang::object, compile_error>;

	return primary_expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[](const ast::literal_expression& literal) -> result_type {
			return evaluate_literal(literal);
		},
		[&](const ast::array_expression& array) {
			return evaluate_array(array, symbols, item_price_data);
		},
		[&](const ast::function_call& function_call) {
			return evaluate_function_call(function_call, symbols, item_price_data);
		},
		[&](const ast::price_range_query& price_range_query) {
			return evaluate_price_range_query(price_range_query, symbols, item_price_data);
		},
		[&](const ast::identifier& identifier) {
			return evaluate_identifier(identifier, symbols);
		},
		[&](const ast::compound_action_expression& expr) {
			return evaluate_compound_action(expr, symbols, item_price_data);
		}
	));
}

} // namespace

namespace fs::compiler::detail
{

std::variant<lang::object, compile_error>
evaluate_value_expression(
	const ast::value_expression& value_expression,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	using result_type = std::variant<lang::object, compile_error>;

	result_type result = evaluate_primary_expression(value_expression.primary_expr, symbols, item_price_data);
	if (std::holds_alternative<compile_error>(result))
		return std::get<compile_error>(std::move(result));

	auto& obj = std::get<lang::object>(result);
	for (auto& expr : value_expression.postfix_exprs)
	{
		result_type res = evaluate_subscript(obj, expr.expr, symbols, item_price_data);
		if (std::holds_alternative<compile_error>(res))
			return std::get<compile_error>(std::move(res));
		else
			obj = std::get<lang::object>(std::move(res));
	}

	return obj;
}

}
