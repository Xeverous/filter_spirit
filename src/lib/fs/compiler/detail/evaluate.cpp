#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/type_constructors.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/lang/queries.hpp>
#include <fs/lang/price_range.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/utility/holds_alternative.hpp>

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

[[nodiscard]] lang::primitive_object
evaluate_literal(const ast::sf::literal_expression& expression)
{
	using result_type = lang::primitive_object_variant;

	auto value = expression.apply_visitor(x3::make_lambda_visitor<result_type>(
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

	return lang::primitive_object{std::move(value), parser::get_position_info(expression)};
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

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_sequence(
	const ast::sf::sequence& sequence,
	const lang::symbol_table& symbols)
{
	assert(!sequence.empty());

	const auto sequence_origin = parser::get_position_info(sequence);

	/*
	 * If sequence has just 1 element and it is an identifier it can be a compound action.
	 * If there are more elements the identifier must refer to a primitive type - compound
	 * actions can not be mixed with primitive elements.
	 */
	if (sequence.size() == 1u) {
		using result_type = std::variant<lang::object, compile_error>;

		return sequence[0].apply_visitor(x3::make_lambda_visitor<result_type>(
			[&](const ast::sf::literal_expression& literal) -> result_type {
				return lang::object{
					lang::sequence_object{
						{ evaluate_literal(literal) },
						sequence_origin
					},
					sequence_origin
				};
			},
			[&](const ast::sf::identifier& identifier) {
				return evaluate_identifier(identifier, symbols);
			}
		));
	}

	/*
	 * There is more than 1 element - all must be primitive.
	 */
	lang::sequence_object::container_type seq_values;
	for (const ast::sf::primitive_value& primitive : sequence) {
		if (holds_alternative<ast::sf::literal_expression>(primitive.var)) {
			seq_values.push_back(evaluate_literal(boost::get<ast::sf::literal_expression>(primitive.var)));
		}

		assert(holds_alternative<ast::sf::identifier>(primitive.var));
		const auto& iden = boost::get<ast::sf::identifier>(primitive.var);

		std::variant<lang::object, compile_error> obj_or_err = evaluate_identifier(iden, symbols);
		if (holds_alternative<compile_error>(obj_or_err)) {
			return std::get<compile_error>(std::move(obj_or_err));
		}

		auto& obj = std::get<lang::object>(obj_or_err);
		if (!obj.is_sequence()) {
			return errors::type_mismatch{
				lang::object_type::sequence,
				obj.type(),
				obj.origin
			};
		}

		/*
		 * Identifier refers to a sequence - flatten it to the parent sequence
		 * append elements from this sequence in the same order.
		 * During flatteting, intentionally lose the seq origin - the parent one
		 * will be more appropriate for any diagnostic messages.
		 */
		auto& seq = std::get<lang::sequence_object>(obj.value).values;
		std::move(seq.begin(), seq.end(), std::back_inserter(seq_values));
	}

	return lang::object{
		lang::sequence_object{std::move(seq_values), sequence_origin},
		sequence_origin
	};
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
		[&](const ast::sf::sequence& seq) {
			return evaluate_sequence(seq, symbols);
		},
		[](const ast::sf::query& query) -> result_type {
			return lang::object{query.q, parser::get_position_info(query)};
		},
		[&](const ast::sf::compound_action_expression& expr) {
			return evaluate_compound_action(expr, symbols, item_price_data);
		}
	));
}

}
