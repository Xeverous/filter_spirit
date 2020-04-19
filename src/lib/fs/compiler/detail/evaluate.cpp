#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/lang/queries.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/lang/limits.hpp>
#include <fs/utility/holds_alternative.hpp>
#include <fs/utility/assert.hpp>

#include <utility>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

namespace x3 = boost::spirit::x3;

using namespace fs;
using namespace fs::compiler;
namespace ast = fs::parser::ast;

namespace
{

// ---- generic helpers ----

[[nodiscard]] std::optional<compile_error>
expect_integer_in_range(
	lang::integer int_obj,
	int min_allowed_value,
	std::optional<int> max_allowed_value)
{
	const auto val = int_obj.value;
	if (val < min_allowed_value || (max_allowed_value && val > *max_allowed_value))
		return errors::invalid_integer_value{min_allowed_value, max_allowed_value, val, int_obj.origin};

	return std::nullopt;
}

// ---- spirit filter ----

[[nodiscard]] std::variant<lang::single_object, compile_error>
evaluate_literal(const ast::sf::literal_expression& expression)
{
	using result_type = std::variant<lang::single_object, compile_error>;
	using parser::position_tag_of;
	const auto expr_origin = position_tag_of(expression);

	return expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](ast::sf::temp_literal literal) -> result_type {
			return lang::single_object{lang::temp{position_tag_of(literal)}, expr_origin};
		},
		[&](ast::sf::none_literal literal) -> result_type {
			return lang::single_object{lang::none{position_tag_of(literal)}, expr_origin};
		},
		[&](ast::sf::socket_spec_literal literal) -> result_type {
			std::variant<lang::socket_spec, compile_error> ss_or_err =
				detail::evaluate_socket_spec_literal(literal.socket_count, literal.socket_colors);

			if (std::holds_alternative<compile_error>(ss_or_err)) {
				return std::get<compile_error>(std::move(ss_or_err));
			}

			return lang::single_object{std::get<lang::socket_spec>(ss_or_err), expr_origin};
		},
		[&](ast::sf::boolean_literal literal) -> result_type {
			return lang::single_object{lang::boolean{literal.value, position_tag_of(literal)}, expr_origin};
		},
		[&](ast::sf::integer_literal literal) -> result_type {
			return lang::single_object{lang::integer{literal.value, position_tag_of(literal)}, expr_origin};
		},
		[&](ast::sf::floating_point_literal literal) -> result_type {
			return lang::single_object{lang::fractional{literal.value, position_tag_of(literal)}, expr_origin};
		},
		[&](ast::sf::rarity_literal literal) -> result_type {
			return lang::single_object{lang::rarity{literal.value, position_tag_of(literal)}, expr_origin};
		},
		[&](ast::sf::shape_literal literal) -> result_type {
			return lang::single_object{lang::shape{literal.value, position_tag_of(literal)}, expr_origin};
		},
		[&](ast::sf::suit_literal literal) -> result_type {
			return lang::single_object{lang::suit{literal.value, position_tag_of(literal)}, expr_origin};
		},
		[&](ast::sf::influence_literal literal) -> result_type {
			return lang::single_object{lang::influence{literal.value, position_tag_of(literal)}, expr_origin};
		},
		[&](const ast::sf::string_literal& literal) -> result_type {
			return lang::single_object{lang::string{literal, position_tag_of(literal)}, expr_origin};
		}
	));
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_name(
	const ast::sf::name& name,
	const lang::symbol_table& symbols)
{
	const auto origin = parser::position_tag_of(name);

	const auto it = symbols.find(name.value.value);
	if (it == symbols.end())
		return errors::no_such_name{origin};

	return lang::object{it->second.object_instance.values, origin};
}

[[nodiscard]] std::variant<lang::object, compile_error>
evaluate_compound_action(
	const ast::sf::compound_action_expression& expr,
	const lang::symbol_table& symbols)
{
	lang::action_set set;

	for (const ast::sf::action& act : expr) {
		std::optional<compile_error> error = detail::spirit_filter_add_action(act, symbols, set);

		if (error)
			return *std::move(error);
	}

	const auto origin = parser::position_tag_of(expr);

	return lang::object{
		lang::object::container_type{lang::single_object{std::move(set), origin}},
		origin
	};
}

} // namespace

namespace fs::compiler::detail
{

std::variant<lang::color, compile_error>
make_color(
	lang::integer r,
	lang::integer g,
	lang::integer b,
	std::optional<lang::integer> a)
{
	std::optional<compile_error> err = expect_integer_in_range(r, 0, 255);
	if (err) {
		return *std::move(err);
	}

	err = expect_integer_in_range(g, 0, 255);
	if (err) {
		return *std::move(err);
	}

	err = expect_integer_in_range(b, 0, 255);
	if (err) {
		return *std::move(err);
	}

	if (a) {
		err = expect_integer_in_range(*a, 0, 255);
		if (err) {
			return *std::move(err);
		}

		return lang::color{r, g, b, *a};
	}

	return lang::color{r, g, b};
}

std::variant<lang::socket_spec, compile_error>
make_socket_spec(
	const std::string& raw,
	lang::position_tag origin)
{
	if (raw.empty())
		return errors::empty_socket_spec{origin};

	lang::socket_spec ss;
	for (char c : raw) {
		namespace kw = lang::keywords::rf;

		if (c == kw::r)
			++ss.r;
		else if (c == kw::g)
			++ss.g;
		else if (c == kw::b)
			++ss.b;
		else if (c == kw::w)
			++ss.w;
		else if (c == kw::a)
			++ss.a;
		else if (c == kw::d)
			++ss.d;
		else
			return errors::illegal_characters_in_socket_spec{origin};
	}

	if (!ss.is_valid())
		return errors::invalid_socket_spec{origin};

	ss.origin = origin;
	return ss;
}

std::variant<lang::builtin_alert_sound, compile_error>
make_builtin_alert_sound(
	bool positional,
	lang::integer sound_id,
	std::optional<lang::integer> volume)
{
	std::optional<compile_error> err = expect_integer_in_range(sound_id, 1, 16);
	if (err) {
		return *std::move(err);
	}

	if (volume) {
		err = expect_integer_in_range(*volume, 0, 300);
		if (err) {
			return *std::move(err);
		}

		return lang::builtin_alert_sound{positional, sound_id, *volume};
	}

	return lang::builtin_alert_sound{positional, sound_id};
}

std::variant<lang::socket_spec, compile_error>
evaluate_socket_spec_literal(
	boost::optional<parser::ast::common::integer_literal> int_lit,
	const parser::ast::common::identifier& iden)
{
	if (int_lit) {
		auto& socket_count = *int_lit;

		if (socket_count.value < lang::limits::min_item_sockets
			|| socket_count.value > lang::limits::max_item_sockets)
		{
			return errors::invalid_integer_value{
				lang::limits::min_item_sockets,
				lang::limits::max_item_sockets,
				socket_count.value,
				parser::position_tag_of(socket_count)
			};
		}
	}

	std::variant<lang::socket_spec, compile_error> ss_or_err =
		detail::make_socket_spec(iden.value, parser::position_tag_of(iden));

	if (std::holds_alternative<compile_error>(ss_or_err)) {
		return std::get<compile_error>(std::move(ss_or_err));
	}

	// TODO adjust origin to also contain the integer literal
	auto& ss = std::get<lang::socket_spec>(ss_or_err);
	if (int_lit)
		ss.num = (*int_lit).value;

	return ss;
}

std::variant<lang::object, compile_error>
evaluate_sequence(
	const ast::sf::sequence& sequence,
	const lang::symbol_table& symbols,
	int min_allowed_elements,
	std::optional<int> max_allowed_elements)
{
	BOOST_ASSERT(!sequence.empty());

	lang::object::container_type seq_values;
	seq_values.reserve(sequence.size());
	for (const ast::sf::primitive_value& primitive : sequence) {
		using result_type = std::variant<lang::object, compile_error>;

		auto res = primitive.apply_visitor(x3::make_lambda_visitor<result_type>(
			[&](const ast::sf::literal_expression& expr) -> result_type {
				std::variant<lang::single_object, compile_error> sobj_or_err = evaluate_literal(expr);
				if (std::holds_alternative<compile_error>(sobj_or_err)) {
					return std::get<compile_error>(sobj_or_err);
				}

				return lang::object{
					{std::get<lang::single_object>(std::move(sobj_or_err))},
					parser::position_tag_of(expr)
				};
			},
			[&](const ast::sf::name& name) {
				return evaluate_name(name, symbols);
			}
		));

		if (std::holds_alternative<compile_error>(res))
			return std::get<compile_error>(std::move(res));

		auto& obj = std::get<lang::object>(res);
		/*
		 * Flatten the sequence to the parent sequence,
		 * append elements from this sequence in the same order.
		 * During flatteting, intentionally lose the seq origin - the
		 * parent one will be more appropriate for any diagnostic messages.
		 */
		std::move(obj.values.begin(), obj.values.end(), std::back_inserter(seq_values));
	}

	const auto sequence_origin = parser::position_tag_of(sequence);
	const auto values_num = static_cast<int>(seq_values.size());

	if (values_num < min_allowed_elements
		|| (max_allowed_elements && values_num > *max_allowed_elements))
	{
		return errors::invalid_amount_of_arguments{
			min_allowed_elements, max_allowed_elements, values_num, sequence_origin
		};
	}

	return lang::object{std::move(seq_values), sequence_origin};
}

std::variant<lang::object, compile_error>
evaluate_value_expression(
	const ast::sf::value_expression& value_expression,
	const lang::symbol_table& symbols)
{
	using result_type = std::variant<lang::object, compile_error>;

	return value_expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](const ast::sf::sequence& seq) {
			return evaluate_sequence(seq, symbols);
		},
		[&](const ast::sf::compound_action_expression& expr) {
			return evaluate_compound_action(expr, symbols);
		}
	));
}

[[nodiscard]] std::variant<lang::fractional, compile_error>
get_as_fractional(const lang::single_object& sobj)
{
	auto& val = sobj.value;

	if (std::holds_alternative<lang::fractional>(val)) {
		return std::get<lang::fractional>(val);
	}

	if (std::holds_alternative<lang::integer>(val)) {
		const auto& intgr = std::get<lang::integer>(val);
		return lang::fractional{static_cast<double>(intgr.value), intgr.origin};
	}

	return errors::type_mismatch{lang::object_type::fractional, sobj.type(), sobj.origin};
}

std::variant<lang::socket_spec, compile_error>
get_as_socket_spec(
	const lang::single_object& sobj)
{
	auto& val = sobj.value;

	if (std::holds_alternative<lang::socket_spec>(val)) {
		return std::get<lang::socket_spec>(val);
	}

	if (std::holds_alternative<lang::integer>(val)) {
		const auto& intgr = std::get<lang::integer>(val);
		lang::socket_spec ss;
		ss.num = intgr.value;
		ss.origin = intgr.origin;
		return ss;
	}

	return errors::type_mismatch{lang::object_type::socket_spec, sobj.type(), sobj.origin};
}

} // namespace fs::compiler::detail
