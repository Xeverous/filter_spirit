#include <fs/compiler/detail/evaluate.hpp>
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
using dmid = diagnostic_message_id;

namespace
{

// ---- generic helpers ----

// (none)

// ---- spirit filter ----

[[nodiscard]] boost::optional<lang::object>
evaluate_name_as_object(
	const ast::sf::name& name,
	const symbol_table& symbols,
	diagnostics_store& diagnostics)
{
	const auto name_origin = parser::position_tag_of(name);

	const auto it = symbols.objects.find(name.value.value);
	if (it == symbols.objects.end()) {
		if (const auto tit = symbols.trees.find(name.value.value); tit == symbols.trees.end()) {
			diagnostics.push_error_no_such_name(name_origin);
		}
		else {
			diagnostics.push_message(make_error(
				dmid::type_mismatch,
				name_origin,
				"type mismatch in expression, expected primitive language type but got subtree object"));
			diagnostics.push_message(make_note_minor(
				boost::none,
				"if you want to assign a subtree write \"$y = { Expand $x }\""));
		}

		return boost::none;
	}

	/*
	 * Named objects should have origin pointing to the name;
	 * this is much better for any error messages than pointing to
	 * the origin of the value, which could be defined many assignments upwards.
	 */
	return lang::object{it->second.object_instance.values, name_origin};
}

} // namespace

namespace fs::compiler::detail
{

// this could actually return something like bounded_integer<Min, Max>
// so far all use cases provide constexpr min/max values
boost::optional<lang::integer>
make_integer_in_range(
	lang::integer int_obj,
	int min_allowed_value,
	int max_allowed_value,
	diagnostics_store& diagnostics)
{
	const auto val = int_obj.value;
	if (val < min_allowed_value || val > max_allowed_value) {
		diagnostics.push_error_invalid_integer_value(
			min_allowed_value,
			max_allowed_value,
			int_obj);
		return boost::none;
	}

	return int_obj;
}

boost::optional<lang::socket_spec>
evaluate(
	settings /* st */,
	const parser::ast::common::socket_spec_literal& literal,
	diagnostics_store& diagnostics)
{
	lang::socket_spec ss;
	ss.origin = parser::position_tag_of(literal.socket_colors);

	if (literal.socket_count) {
		const auto& intgr = evaluate(*literal.socket_count);

		if (intgr.value < lang::limits::min_item_sockets
			|| intgr.value > lang::limits::max_item_sockets)
		{
			diagnostics.push_error_invalid_integer_value(
				lang::limits::min_item_sockets,
				lang::limits::max_item_sockets,
				intgr);
		}

		ss.num = intgr.value;
		ss.origin = lang::merge_origins(intgr.origin, ss.origin);
	}

	const std::string& raw_letters = literal.socket_colors.value;
	if (raw_letters.empty()) {
		/*
		 * This should be considered internal error because:
		 * 1. If integer literal is present, the sole integer should be parsed
		 *    as integer literal instead and should not reach this evaluate function.
		 * 2. If integer literal is not present, then both tokens are empty so
		 *    something must have gone really bad with whitespace grammar.
		 */
		diagnostics.push_error_internal_compiler_error(__func__, parser::position_tag_of(literal.socket_colors));
		return boost::none;
	}

	for (std::size_t i = 0; i < raw_letters.size(); ++i) {
		namespace kw = lang::keywords::rf;
		const auto c = raw_letters[i];

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
		else {
			diagnostics.push_message(make_error(
				dmid::illegal_character_in_socket_spec,
				parser::position_tag_of(literal.socket_colors),
				"illegal character in socket group at position ",
				std::to_string(i),
				" (only R/G/B/W/A/D characters are allowed)"));
		}
	}

	if (!ss.is_valid()) {
		diagnostics.push_message(make_error(dmid::invalid_socket_spec, ss.origin, "invalid socket group"));
		return boost::none; // return nothing, flow can not go further with invalid socket spec
	}

	return ss;
}

boost::optional<lang::single_object>
evaluate(
	settings st,
	const ast::common::literal_expression& expression,
	diagnostics_store& diagnostics)
{
	using result_type = boost::optional<lang::single_object>;
	using parser::position_tag_of;
	const auto expr_origin = position_tag_of(expression);

	return expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](const auto& literal) {
			return lang::single_object{evaluate(literal), expr_origin};
		},
		[&](ast::common::socket_spec_literal literal) -> result_type {
			return detail::evaluate(st, literal, diagnostics)
				.map([&](lang::socket_spec ss) {
					return lang::single_object{ss, expr_origin};
				});
		}
	));
}

boost::optional<lang::object>
evaluate_sequence(
	settings st,
	const ast::sf::sequence& sequence,
	const symbol_table& symbols,
	int min_allowed_elements,
	boost::optional<int> max_allowed_elements,
	diagnostics_store& diagnostics)
{
	FS_ASSERT(!sequence.empty());

	lang::object::container_type seq_values;
	seq_values.reserve(sequence.size());

	for (const ast::sf::primitive_value& primitive : sequence) {
		using result_type = boost::optional<lang::object>;

		auto result = primitive.apply_visitor(x3::make_lambda_visitor<result_type>(
			[&](const ast::sf::name& name) {
				return evaluate_name_as_object(name, symbols, diagnostics);
			},
			[&](const ast::common::literal_expression& expr) -> result_type {
				return evaluate(st, expr, diagnostics)
					.map([&](lang::single_object so) {
						return lang::object{{std::move(so)}, parser::position_tag_of(expr)};
					}
				);
			},
			[&](const ast::common::unknown_expression& expr) -> result_type {
				diagnostics.push_error_unknown_expression(parser::position_tag_of(expr));
				return boost::none;
			}
		));

		if (result) {
			lang::object& obj = *result;
			/*
			 * Flatten the sequence to the parent sequence,
			 * append elements from this sequence in the same order.
			 * During flatteting, intentionally lose the seq origin - the
			 * parent one will be more appropriate for any diagnostic messages.
			 */
			std::move(obj.values.begin(), obj.values.end(), std::back_inserter(seq_values));
		}
		else if (st.error_handling.stop_on_error) {
			return boost::none;
		}
	}

	const auto sequence_origin = parser::position_tag_of(sequence);
	const auto num_elements = static_cast<int>(seq_values.size());

	if (num_elements < min_allowed_elements
		|| (max_allowed_elements && num_elements > *max_allowed_elements))
	{
		// It is possible to return an object here, but there are functions
		// that expect the result of this function to match required min/max
		// elements. Therefore, an error is added and no object is returned.
		diagnostics.push_error_invalid_amount_of_arguments(
			min_allowed_elements, max_allowed_elements, num_elements, sequence_origin);
		return boost::none;
	}

	return lang::object{std::move(seq_values), sequence_origin};
}

boost::optional<lang::object>
evaluate_literal_sequence(
	settings st,
	const parser::ast::rf::literal_sequence& sequence,
	int min_allowed_elements,
	boost::optional<int> max_allowed_elements,
	diagnostics_store& diagnostics)
{
	FS_ASSERT(!sequence.empty());

	lang::object::container_type seq_values;
	seq_values.reserve(sequence.size());

	for (const ast::common::literal_expression& literal_expr : sequence) {
		// lang::single_object is not ideal here, because it supports more than literals
		// but other code in real filter's compiler logic will simply error on non-literals
		boost::optional<lang::single_object> so = evaluate(st, literal_expr, diagnostics);

		if (so)
			seq_values.push_back(std::move(*so));
		else if (st.error_handling.stop_on_error)
			return boost::none;
	}

	const auto sequence_origin = parser::position_tag_of(sequence);
	const auto num_elements = static_cast<int>(seq_values.size());

	if (num_elements < min_allowed_elements
		|| (max_allowed_elements && num_elements > *max_allowed_elements))
	{
		// It is possible to return an object here, but there are functions
		// that expect the result of this function to match required min/max
		// elements. Therefore, an error is added and no object is returned.
		diagnostics.push_error_invalid_amount_of_arguments(
			min_allowed_elements, max_allowed_elements, num_elements, sequence_origin);
		return boost::none;
	}

	return lang::object{std::move(seq_values), sequence_origin};
}

boost::optional<lang::fractional>
get_as_fractional(
	const lang::single_object& sobj,
	diagnostics_store& diagnostics)
{
	auto& val = sobj.value;

	if (std::holds_alternative<lang::fractional>(val)) {
		return std::get<lang::fractional>(val);
	}

	if (std::holds_alternative<lang::integer>(val)) {
		const auto& intgr = std::get<lang::integer>(val);
		return lang::fractional{static_cast<double>(intgr.value), intgr.origin};
	}

	diagnostics.push_error_type_mismatch(lang::object_type::fractional, sobj.type(), sobj.origin);
	return boost::none;
}

boost::optional<lang::socket_spec>
get_as_socket_spec(
	const lang::single_object& sobj,
	diagnostics_store& diagnostics)
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

	diagnostics.push_error_type_mismatch(lang::object_type::socket_spec, sobj.type(), sobj.origin);
	return boost::none;
}

} // namespace fs::compiler::detail
