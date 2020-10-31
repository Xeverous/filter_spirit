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

// this could actually return something like bounded_integer<Min, Max>
// so far all use cases provide constexpr min/max values
[[nodiscard]] outcome<lang::integer>
make_integer_in_range(
	lang::integer int_obj,
	int min_allowed_value,
	std::optional<int> max_allowed_value)
{
	const auto val = int_obj.value;
	if (val < min_allowed_value || (max_allowed_value && val > *max_allowed_value))
		return error(errors::invalid_integer_value{min_allowed_value, max_allowed_value, val, int_obj.origin});

	return int_obj;
}

// ---- spirit filter ----

[[nodiscard]] outcome<lang::single_object>
evaluate_literal(
	settings st,
	const ast::sf::literal_expression& expression)
{
	using result_type = outcome<lang::single_object>;
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
			return detail::evaluate_socket_spec_literal(st, literal.socket_count, literal.socket_colors)
				.map_result<lang::single_object>([&](lang::socket_spec ss) {
					return lang::single_object{ss, expr_origin};
				});
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
		[&](ast::sf::shaper_voice_line_literal literal) -> result_type {
			return lang::single_object{lang::shaper_voice_line{literal.value, position_tag_of(literal)}, expr_origin};
		},
		[&](ast::sf::gem_quality_type_literal literal) -> result_type {
			return lang::single_object{lang::gem_quality_type{literal.value, position_tag_of(literal)}, expr_origin};
		},
		[&](const ast::sf::string_literal& literal) -> result_type {
			return lang::single_object{lang::string{literal, position_tag_of(literal)}, expr_origin};
		}
	));
}

[[nodiscard]] outcome<lang::object>
evaluate_name(
	const ast::sf::name& name,
	const lang::symbol_table& symbols)
{
	const auto name_origin = parser::position_tag_of(name);

	const auto it = symbols.find(name.value.value);
	if (it == symbols.end())
		return error(errors::no_such_name{name_origin});

	/*
	 * Named objects should have origin pointing to the name;
	 * this is much better for any error messages than pointing to
	 * the origin of the value, which could be defined may assignments upwards.
	 */
	return lang::object{it->second.object_instance.values, name_origin};
}

[[nodiscard]] outcome<lang::object>
evaluate_compound_action(
	settings st,
	const ast::sf::compound_action_expression& expr,
	const lang::symbol_table& symbols)
{
	lang::action_set set;
	log_container logs;

	for (const ast::sf::action& act : expr) {
		detail::spirit_filter_add_action(st, act, symbols, set).move_logs_to(logs);

		if (!should_continue(st.error_handling, logs))
			return logs;
	}

	const auto origin = parser::position_tag_of(expr);

	return {
		lang::object{
			lang::object::container_type{lang::single_object{std::move(set), origin}},
			origin
		},
		std::move(logs)
	};
}

} // namespace

namespace fs::compiler::detail
{

outcome<lang::color>
make_color(
	settings /* st */,
	lang::integer r,
	lang::integer g,
	lang::integer b,
	std::optional<lang::integer> a)
{
	auto rgb = make_integer_in_range(r, 0, 255)
		.merge_with(make_integer_in_range(g, 0, 255))
		.merge_with(make_integer_in_range(b, 0, 255));

	if (a) {
		return std::move(rgb)
			.merge_with(make_integer_in_range(*a, 0, 255))
			.map_result<lang::color>([](lang::integer r, lang::integer g, lang::integer b, lang::integer a) {
				return lang::color{r, g, b, a};
			});
	}
	else {
		return std::move(rgb)
			.map_result<lang::color>([](lang::integer r, lang::integer g, lang::integer b) {
				return lang::color{r, g, b};
			});
	}
}

// TODO this does not make a full socket_spec, only the RGBWAD subset
// either make a subtype representing number-less socket specs or refactor this function
outcome<lang::socket_spec>
make_socket_spec(
	settings st,
	const std::string& raw,
	lang::position_tag origin)
{
	if (raw.empty()) {
		// nothing more to do at this point, return failure immediately
		return error(errors::empty_socket_spec{origin});
	}

	lang::socket_spec ss;
	log_container logs;

	for (std::size_t i = 0; i < raw.size(); ++i) {
		namespace kw = lang::keywords::rf;
		const auto c = raw[i];

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
			logs.emplace_back(error(errors::illegal_character_in_socket_spec{origin, static_cast<int>(i)}));

		if (!should_continue(st.error_handling, logs))
			return logs;
	}

	if (!ss.is_valid()) {
		// return failure now, can not work go further with invalid socket spec
		logs.emplace_back(error(errors::invalid_socket_spec{origin}));
		return logs;
	}

	ss.origin = origin;
	return {ss, std::move(logs)};
}

outcome<lang::builtin_alert_sound_id>
make_builtin_alert_sound_id(
	lang::integer sound_id)
{
	return make_integer_in_range(sound_id, lang::limits::min_filter_sound_id, lang::limits::max_filter_sound_id)
		.map_result<lang::builtin_alert_sound_id>([](lang::integer sound_id) {
			return lang::builtin_alert_sound_id{sound_id};
		});
}

outcome<lang::builtin_alert_sound>
make_builtin_alert_sound(
	settings /* st */,
	bool positional,
	lang::builtin_alert_sound_id sound_id,
	std::optional<lang::integer> volume)
{
	if (volume) {
		return make_integer_in_range(*volume, lang::limits::min_filter_volume, lang::limits::max_filter_volume)
			.map_result<lang::builtin_alert_sound>([&](lang::integer volume) {
				return lang::builtin_alert_sound{positional, sound_id, volume};
			});
	}
	else {
		return lang::builtin_alert_sound{positional, sound_id};
	}
}

// TODO improve error handling in this function
// refactor so that int_lit is checked once
outcome<lang::socket_spec>
evaluate_socket_spec_literal(
	settings st,
	boost::optional<ast::common::integer_literal> int_lit,
	const parser::ast::common::identifier& iden)
{
	if (int_lit) {
		auto& socket_count = *int_lit;

		if (socket_count.value < lang::limits::min_item_sockets
			|| socket_count.value > lang::limits::max_item_sockets)
		{
			return error{errors::invalid_integer_value{
				lang::limits::min_item_sockets,
				lang::limits::max_item_sockets,
				socket_count.value,
				parser::position_tag_of(socket_count)}};
		}
	}

	return detail::make_socket_spec(st, iden.value, parser::position_tag_of(iden))
		.map_result<lang::socket_spec>([&](lang::socket_spec ss) {
			// TODO adjust origin to also contain the integer literal
			if (int_lit)
				ss.num = (*int_lit).value;

			return ss;
		});
}

outcome<lang::object>
evaluate_sequence(
	settings st,
	const ast::sf::sequence& sequence,
	const lang::symbol_table& symbols,
	int min_allowed_elements,
	std::optional<int> max_allowed_elements)
{
	FS_ASSERT(!sequence.empty());

	lang::object::container_type seq_values;
	seq_values.reserve(sequence.size());

	log_container log;

	for (const ast::sf::primitive_value& primitive : sequence) {
		using result_type = outcome<lang::object>;

		primitive.apply_visitor(x3::make_lambda_visitor<result_type>(
			[&](const ast::sf::literal_expression& expr) -> result_type {
				return evaluate_literal(st, expr).map_result<lang::object>(
					[&](lang::single_object so) {
						return lang::object{{std::move(so)}, parser::position_tag_of(expr)};
					}
				);
			},
			[&](const ast::sf::name& name) {
				return evaluate_name(name, symbols);
			}
		))
		.map_result([&](lang::object obj) {
			/*
			 * Flatten the sequence to the parent sequence,
			 * append elements from this sequence in the same order.
			 * During flatteting, intentionally lose the seq origin - the
			 * parent one will be more appropriate for any diagnostic messages.
			 */
			std::move(obj.values.begin(), obj.values.end(), std::back_inserter(seq_values));
		})
		.move_logs_to(log);

		if (!should_continue(st.error_handling, log))
			return log;
	}

	const auto sequence_origin = parser::position_tag_of(sequence);
	const auto num_elements = static_cast<int>(seq_values.size());

	if (num_elements < min_allowed_elements
		|| (max_allowed_elements && num_elements > *max_allowed_elements))
	{
		// It is possible to return object + log here, but there are functions
		// that expect the result of this function to match required min/max
		// elements. Therefore, only log is returned.
		log.emplace_back(error(errors::invalid_amount_of_arguments{
			min_allowed_elements, max_allowed_elements, num_elements, sequence_origin}));
		return log;
	}

	return {lang::object{std::move(seq_values), sequence_origin}, std::move(log)};
}

outcome<lang::object>
evaluate_value_expression(
	settings st,
	const ast::sf::value_expression& value_expression,
	const lang::symbol_table& symbols)
{
	using result_type = outcome<lang::object>;

	return value_expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](const ast::sf::sequence& seq) {
			return evaluate_sequence(st, seq, symbols);
		},
		[&](const ast::sf::compound_action_expression& expr) {
			return evaluate_compound_action(st, expr, symbols);
		}
	));
}

[[nodiscard]] outcome<lang::fractional>
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

	return error(errors::type_mismatch{lang::object_type::fractional, sobj.type(), sobj.origin});
}

outcome<lang::socket_spec>
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

	return error(errors::type_mismatch{lang::object_type::socket_spec, sobj.type(), sobj.origin});
}

} // namespace fs::compiler::detail
