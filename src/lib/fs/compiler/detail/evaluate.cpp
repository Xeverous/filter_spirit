#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/evaluate_as.hpp>
#include <fs/compiler/detail/get_value_as.hpp>
#include <fs/compiler/detail/queries.hpp>
#include <fs/compiler/detail/construct.hpp>
#include <fs/compiler/detail/type_constructors.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/lang/functions.hpp>
#include <fs/lang/queries.hpp>
#include <fs/lang/price_range.hpp>
#include <fs/lang/position_tag.hpp>

#include <cassert>
#include <utility>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

namespace x3 = boost::spirit::x3;

using namespace fs;
using namespace fs::compiler;
namespace ast = fs::parser::ast;

namespace
{

// ---- real filter ----

// ---- spirit filter ----

[[nodiscard]] lang::object
evaluate_literal(const ast::sf::literal_expression& expression)
{
	using result_type = lang::object_variant;

	auto object = expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[](ast::sf::underscore_literal) {
			return lang::underscore{};
		},
		[](ast::sf::boolean_literal literal) -> result_type {
			return lang::boolean{literal.value};
		},
		[](ast::sf::integer_literal literal) -> result_type {
			return lang::integer{literal.value};
		},
		[](ast::sf::floating_point_literal literal) -> result_type {
			return lang::floating_point{literal.value};
		},
		[](ast::sf::rarity_literal literal) -> result_type {
			return lang::rarity{literal.value};
		},
		[](ast::sf::shape_literal literal) -> result_type {
			return lang::shape{literal.value};
		},
		[](ast::sf::suit_literal literal) -> result_type {
			return lang::suit{literal.value};
		},
		[](ast::sf::influence_literal literal) -> result_type {
			return lang::influence{literal.value};
		},
		[](const ast::sf::string_literal& literal) -> result_type {
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
	const lang::object_type first_type = first_object.type();
	for (const lang::object& element : array) {
		const lang::object_type tested_type = element.type();
		if (tested_type != first_type) { // C++20: [[unlikely]]
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
	const ast::sf::array_expression& expression,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	// note: the entire function should work also in case of empty array
	lang::array_object array;
	for (const ast::sf::value_expression& value_expression : expression.elements) {
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

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_function_call(
	const ast::sf::function_call& function_call,
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
	const ast::sf::identifier& function_name = function_call.name;
	/*
	 * note: this is O(n) but relying on small string optimization
	 * and much better memory layout makes it to run faster than a
	 * tree-based or hash-based map. We can also optimize order of
	 * comparisons.
	 */
	using compiler::detail::construct;
	const auto origin = parser::get_position_info(function_call);
	if (function_name.value == lang::functions::rgb) {
		return detail::generalize_type(construct<lang::color>(function_call, symbols, item_price_data), origin);
	}
	else if (function_name.value == lang::functions::level) {
		return detail::generalize_type(construct<lang::level>(function_call, symbols, item_price_data), origin);
	}
	else if (function_name.value == lang::functions::font_size) {
		return detail::generalize_type(construct<lang::font_size>(function_call, symbols, item_price_data), origin);
	}
	else if (function_name.value == lang::functions::sound_id) {
		return detail::generalize_type(construct<lang::sound_id>(function_call, symbols, item_price_data), origin);
	}
	else if (function_name.value == lang::functions::volume) {
		return detail::generalize_type(construct<lang::volume>(function_call, symbols, item_price_data), origin);
	}
	else if (function_name.value == lang::functions::group) {
		return detail::generalize_type(construct<lang::socket_group>(function_call, symbols, item_price_data), origin);
	}
	else if (function_name.value == lang::functions::minimap_icon) {
		return detail::generalize_type(construct<lang::minimap_icon>(function_call, symbols, item_price_data), origin);
	}
	else if (function_name.value == lang::functions::beam_effect) {
		return detail::generalize_type(construct<lang::beam_effect>(function_call, symbols, item_price_data), origin);
	}
	else if (function_name.value == lang::functions::path) {
		return detail::generalize_type(construct<lang::path>(function_call, symbols, item_price_data), origin);
	}
	else if (function_name.value == lang::functions::alert_sound) {
		return detail::generalize_type(construct<lang::alert_sound>(function_call, symbols, item_price_data), origin);
	}

	return errors::no_such_function{parser::get_position_info(function_name)};
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_price_range_query(
	const ast::sf::price_range_query& price_range_query,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	std::variant<lang::price_range, compile_error> range_or_error =
		compiler::detail::construct_price_range(price_range_query.arguments, symbols, item_price_data);
	if (std::holds_alternative<compile_error>(range_or_error))
		return std::get<compile_error>(std::move(range_or_error));

	const auto& price_range = std::get<lang::price_range>(range_or_error);
	const lang::position_tag position_of_query = parser::get_position_info(price_range_query);

	// TODO this is too complex - move the code to item_price_data when the interface of it
	// is decided upon (think what to do with is_low_confidence). Right now there are no
	// invariants in item_price_data so we do a lot of find/for-each algorithms.
	const auto eval_query = [&](const auto& items, auto price_func, auto name_func) {
		lang::array_object array;
		for (auto it = items.begin(); it != items.end(); ++it) {
			if (price_range.contains(price_func(it))) {
				array.push_back(lang::object{lang::string{name_func(it)}, position_of_query});
			}
		}
		return lang::object{std::move(array), position_of_query};
	};
	const auto eval_query_elementary_items = [&](const auto& items) {
		return eval_query(items, [](auto it) { return it->price.chaos_value; }, [](auto it) { return it->name; });
	};
	const auto eval_query_unamb_unique_items = [&](const auto& items) {
		return eval_query(items, [](auto it) { return it->second.price.chaos_value; }, [](auto it) { return it->first; });
	};
	const auto eval_query_amb_unique_items = [&](const auto& items) {
		return eval_query(
			items,
			[](auto it) {
				auto& items = it->second;
				return std::max_element(
					items.begin(),
					items.end(),
					[](const auto& lhs, const auto& rhs) {
						return lhs.price.chaos_value < rhs.price.chaos_value;
					})->price.chaos_value;
			},
			[](auto it) { return it->first; });
	};
	/*
	 * note: this is O(n) but relying on small string optimization
	 * and much better memory layout makes it to run faster than a
	 * tree-based or hash-based map. We can also optimize order of
	 * comparisons.
	 */
	const ast::sf::identifier& query_name = price_range_query.name;
	if (query_name.value == lang::queries::divination) {
		return eval_query_elementary_items(item_price_data.divination_cards); // TODO use complex query later
	}
	else if (query_name.value == lang::queries::oils) {
		return eval_query_elementary_items(item_price_data.prophecies);
	}
	else if (query_name.value == lang::queries::incubators) {
		return eval_query_elementary_items(item_price_data.prophecies);
	}
	else if (query_name.value == lang::queries::essences) {
		return eval_query_elementary_items(item_price_data.essences);
	}
	else if (query_name.value == lang::queries::fossils) {
		return eval_query_elementary_items(item_price_data.fossils);
	}
	else if (query_name.value == lang::queries::prophecies) {
		return eval_query_elementary_items(item_price_data.prophecies);
	}
	else if (query_name.value == lang::queries::resonators) {
		return eval_query_elementary_items(item_price_data.resonators);
	}
	else if (query_name.value == lang::queries::scarabs) {
		return eval_query_elementary_items(item_price_data.scarabs);
	}
	else if (query_name.value == lang::queries::helmet_enchants) {
		return eval_query_elementary_items(item_price_data.helmet_enchants);
	}
	else if (query_name.value == lang::queries::uniques_eq_ambiguous) {
		return eval_query_amb_unique_items(item_price_data.unique_eq.ambiguous);
	}
	else if (query_name.value == lang::queries::uniques_eq_unambiguous) {
		return eval_query_unamb_unique_items(item_price_data.unique_eq.unambiguous);
	}
	else if (query_name.value == lang::queries::uniques_flask_ambiguous) {
		return eval_query_amb_unique_items(item_price_data.unique_flasks.ambiguous);
	}
	else if (query_name.value == lang::queries::uniques_flask_unambiguous) {
		return eval_query_unamb_unique_items(item_price_data.unique_flasks.unambiguous);
	}
	else if (query_name.value == lang::queries::uniques_jewel_ambiguous) {
		return eval_query_amb_unique_items(item_price_data.unique_jewels.ambiguous);
	}
	else if (query_name.value == lang::queries::uniques_jewel_unambiguous) {
		return eval_query_unamb_unique_items(item_price_data.unique_jewels.unambiguous);
	}
	else if (query_name.value == lang::queries::uniques_map_ambiguous) {
		return eval_query_amb_unique_items(item_price_data.unique_maps.ambiguous);
	}
	else if (query_name.value == lang::queries::uniques_map_unambiguous) {
		return eval_query_unamb_unique_items(item_price_data.unique_maps.unambiguous);
	}

	return errors::no_such_query{parser::get_position_info(query_name)};
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_identifier(
	const ast::sf::identifier& identifier,
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
	const ast::sf::compound_action_expression& expr,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	lang::action_set actions;

	for (const ast::sf::action& act : expr) {
		std::optional<compile_error> error = detail::spirit_filter_add_action(act, symbols, item_price_data, actions);

		if (error)
			return *std::move(error);
	}

	return lang::object{std::move(actions), parser::get_position_info(expr)};
}

} // namespace

namespace fs::compiler::detail
{

std::variant<lang::minimap_icon, compile_error>
evaluate(parser::ast::rf::icon_literal il)
{
	return detail::type_constructor<lang::minimap_icon>::call(
		parser::get_position_info(il),
		{
			parser::get_position_info(il.size),
			parser::get_position_info(il.suit),
			parser::get_position_info(il.shape)
		},
		detail::evaluate(il.size),
		detail::evaluate(il.suit),
		detail::evaluate(il.shape)
	);
}

std::variant<lang::socket_group, compile_error>
evaluate_as_socket_group(const std::string& str, lang::position_tag origin)
{
	return detail::type_constructor<lang::socket_group>::call(
		origin, {origin}, str);
}

std::variant<lang::object, compile_error>
evaluate_value_expression(
	const ast::sf::value_expression& value_expression,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data)
{
	using result_type = std::variant<lang::object, compile_error>;

	return value_expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[](const ast::sf::literal_expression& literal) -> result_type {
			return evaluate_literal(literal);
		},
		[&](const ast::sf::array_expression& array) {
			return evaluate_array(array, symbols, item_price_data);
		},
		[&](const ast::sf::function_call& function_call) {
			return evaluate_function_call(function_call, symbols, item_price_data);
		},
		[&](const ast::sf::price_range_query& price_range_query) {
			return evaluate_price_range_query(price_range_query, symbols, item_price_data);
		},
		[&](const ast::sf::identifier& identifier) {
			return evaluate_identifier(identifier, symbols);
		},
		[&](const ast::sf::compound_action_expression& expr) {
			return evaluate_compound_action(expr, symbols, item_price_data);
		}
	));
}

}
