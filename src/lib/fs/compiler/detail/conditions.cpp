#include <fs/compiler/detail/conditions.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/condition_set.hpp>
#include <fs/lang/limits.hpp>
#include <fs/utility/assert.hpp>

#include <boost/container/static_vector.hpp>
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <utility>

namespace ast = fs::parser::ast;

namespace x3 = boost::spirit::x3;

namespace
{

using namespace fs;
using namespace fs::compiler;

// ---- generic helpers ----

template <typename T>
[[nodiscard]] std::optional<compile_error>
add_range_condition(
	lang::comparison_type comparison_type,
	T value,
	lang::position_tag condition_origin,
	lang::range_condition<T>& target)
{
	switch (comparison_type) {
		case lang::comparison_type::equal: {
			if (target.lower_bound.has_value())
				return errors::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin};

			if (target.upper_bound.has_value())
				return errors::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin};

			target.set_exact(value, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::less: {
			if (target.upper_bound.has_value())
				return errors::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin};

			target.set_upper_bound(value, false, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::less_equal: {
			if (target.upper_bound.has_value())
				return errors::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin};

			target.set_upper_bound(value, true, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::greater: {
			if (target.lower_bound.has_value())
				return errors::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin};

			target.set_lower_bound(value, false, condition_origin);
			return std::nullopt;
		}
		case lang::comparison_type::greater_equal: {
			if (target.lower_bound.has_value())
				return errors::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin};

			target.set_lower_bound(value, true, condition_origin);
			return std::nullopt;
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::add_range_condition,
		condition_origin
	};
}

[[nodiscard]] std::optional<compile_error>
add_numeric_comparison_condition(
	lang::numeric_comparison_condition_property property,
	lang::comparison_type cmp,
	lang::integer intgr,
	lang::position_tag condition_origin,
	lang::condition_set& set)
{
	switch (property) {
		case lang::numeric_comparison_condition_property::item_level: {
			return add_range_condition(cmp, intgr, condition_origin, set.item_level);
		}
		case lang::numeric_comparison_condition_property::drop_level: {
			return add_range_condition(cmp, intgr, condition_origin, set.drop_level);
		}
		case lang::numeric_comparison_condition_property::quality: {
			return add_range_condition(cmp, intgr, condition_origin, set.quality);
		}
		case lang::numeric_comparison_condition_property::links: {
			return add_range_condition(cmp, intgr, condition_origin, set.links);
		}
		case lang::numeric_comparison_condition_property::height: {
			return add_range_condition(cmp, intgr, condition_origin, set.height);
		}
		case lang::numeric_comparison_condition_property::width: {
			return add_range_condition(cmp, intgr, condition_origin, set.width);
		}
		case lang::numeric_comparison_condition_property::stack_size: {
			return add_range_condition(cmp, intgr, condition_origin, set.stack_size);
		}
		case lang::numeric_comparison_condition_property::gem_level: {
			return add_range_condition(cmp, intgr, condition_origin, set.gem_level);
		}
		case lang::numeric_comparison_condition_property::map_tier: {
			return add_range_condition(cmp, intgr, condition_origin, set.map_tier);
		}
		case lang::numeric_comparison_condition_property::area_level: {
			return add_range_condition(cmp, intgr, condition_origin, set.area_level);
		}
		case lang::numeric_comparison_condition_property::corrupted_mods: {
			return add_range_condition(cmp, intgr, condition_origin, set.corrupted_mods);
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::add_numeric_comparison_condition,
		condition_origin
	};
}

[[nodiscard]] std::optional<compile_error>
add_string_array_condition_impl(
	std::vector<std::string> strings,
	bool is_exact_match,
	lang::position_tag condition_origin,
	std::optional<lang::strings_condition>& target)
{
	if (target.has_value())
		return errors::condition_redefinition{condition_origin, (*target).origin};

	target = lang::strings_condition{
		std::move(strings),
		is_exact_match,
		condition_origin,
	};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
add_string_array_condition(
	lang::string_array_condition_property property,
	std::vector<std::string> strings,
	bool exact_match,
	lang::position_tag condition_origin,
	lang::condition_set& set)
{
	switch (property) {
		case lang::string_array_condition_property::class_: {
			return add_string_array_condition_impl(std::move(strings), exact_match, condition_origin, set.class_);
		}
		case lang::string_array_condition_property::base_type: {
			return add_string_array_condition_impl(std::move(strings), exact_match, condition_origin, set.base_type);
		}
		case lang::string_array_condition_property::has_explicit_mod: {
			return add_string_array_condition_impl(std::move(strings), exact_match, condition_origin, set.has_explicit_mod);
		}
		case lang::string_array_condition_property::has_enchantment: {
			return add_string_array_condition_impl(std::move(strings), exact_match, condition_origin, set.has_enchantment);
		}
		case lang::string_array_condition_property::prophecy: {
			return add_string_array_condition_impl(std::move(strings), exact_match, condition_origin, set.prophecy);
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::add_string_array_condition,
		condition_origin
	};
}

using influences_container = boost::container::static_vector<
	lang::influence,
	lang::limits::max_filter_influences
>;

[[nodiscard]] std::optional<compile_error>
add_has_influence_condition(
	influences_container influences,
	bool is_exact_match,
	lang::position_tag origin,
	std::optional<lang::influences_condition>& target)
{
	for (std::size_t i = 0; i < influences.size(); ++i) {
		for (std::size_t j = 0; j < influences.size(); ++j) {
			if (j != i && influences[i] == influences[j]) {
				return errors::duplicate_influence{influences[i].origin, influences[j].origin};
			}
		}
	}

	if (target.has_value())
		return errors::condition_redefinition{origin, (*target).origin};

	const auto contains = [&](lang::influence_type type) {
		for (const auto infl : influences)
			if (infl.value == type)
				return true;

		return false;
	};

	target = lang::influences_condition{
		contains(lang::influence_type::shaper),
		contains(lang::influence_type::elder),
		contains(lang::influence_type::crusader),
		contains(lang::influence_type::redeemer),
		contains(lang::influence_type::hunter),
		contains(lang::influence_type::warlord),
		is_exact_match,
		origin
	};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
add_socket_spec_condition(
	bool links_matter,
	lang::socket_spec_condition condition,
	lang::condition_set& set)
{
	auto& target = links_matter ? set.socket_group : set.sockets;

	if (target.has_value())
		return errors::condition_redefinition{condition.origin, (*target).origin};

	target = std::move(condition);
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
add_boolean_condition_impl(
	lang::boolean boolean,
	lang::position_tag condition_origin,
	std::optional<lang::boolean_condition>& target)
{
	if (target.has_value())
		return errors::condition_redefinition{condition_origin, (*target).origin};

	target = lang::boolean_condition{boolean, condition_origin};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
add_boolean_condition(
	lang::boolean boolean,
	lang::position_tag condition_origin,
	lang::boolean_condition_property property,
	lang::condition_set& condition_set)
{
	switch (property) {
		case lang::boolean_condition_property::identified: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_identified);
		}
		case lang::boolean_condition_property::corrupted: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_corrupted);
		}
		case lang::boolean_condition_property::mirrored: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_mirrored);
		}
		case lang::boolean_condition_property::elder_item: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_elder_item);
		}
		case lang::boolean_condition_property::shaper_item: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_shaper_item);
		}
		case lang::boolean_condition_property::fractured_item: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_fractured_item);
		}
		case lang::boolean_condition_property::synthesised_item: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_synthesised_item);
		}
		case lang::boolean_condition_property::any_enchantment: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_enchanted);
		}
		case lang::boolean_condition_property::shaped_map: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_shaped_map);
		}
		case lang::boolean_condition_property::elder_map: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_elder_map);
		}
		case lang::boolean_condition_property::blighted_map: {
			return add_boolean_condition_impl(boolean, condition_origin, condition_set.is_blighted_map);
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::add_boolean_condition,
		condition_origin
	};
}

// ---- spirit filter helpers ----

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_autogen_condition(
	const ast::sf::autogen_condition& condition,
	lang::spirit_condition_set& set)
{
	const auto condition_origin = parser::position_tag_of(condition);

	if (set.autogen.has_value())
		return errors::condition_redefinition{condition_origin, (*set.autogen).origin};

	set.autogen = lang::autogen_condition{condition.cat_expr.category, condition_origin};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_price_comparison_condition(
	const ast::sf::price_comparison_condition& condition,
	const lang::symbol_table& symbols,
	lang::spirit_condition_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(condition.seq, symbols, 1, 1);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);
	BOOST_ASSERT(obj.values.size() == 1u);

	std::variant<lang::fractional, compile_error> frac_or_err = detail::get_as_fractional(obj.values[0]);
	if (std::holds_alternative<compile_error>(frac_or_err))
		return std::get<compile_error>(std::move(frac_or_err));

	auto& frac = std::get<lang::fractional>(frac_or_err);

	return add_range_condition(
		condition.comparison_type.value,
		frac,
		parser::position_tag_of(condition),
		set.price);
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_rarity_comparison_condition(
	const ast::sf::rarity_comparison_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(condition.seq, symbols, 1, 1);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);
	BOOST_ASSERT(obj.values.size() == 1u);

	std::variant<lang::rarity, compile_error> rar_or_err = detail::get_as<lang::rarity>(obj.values[0]);
	if (std::holds_alternative<compile_error>(rar_or_err))
		return std::get<compile_error>(std::move(rar_or_err));

	auto& rar = std::get<lang::rarity>(rar_or_err);

	return add_range_condition(condition.comparison_type.value, rar, parser::position_tag_of(condition), set.rarity);
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_numeric_comparison_condition(
	const ast::sf::numeric_comparison_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(condition.seq, symbols, 1, 1);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);
	BOOST_ASSERT(obj.values.size() == 1u);

	std::variant<lang::integer, compile_error> int_or_err = detail::get_as<lang::integer>(obj.values[0]);
	if (std::holds_alternative<compile_error>(int_or_err))
		return std::get<compile_error>(std::move(int_or_err));

	auto& intgr = std::get<lang::integer>(int_or_err);
	const auto origin = parser::position_tag_of(condition);
	return add_numeric_comparison_condition(condition.property, condition.comparison_type.value, intgr, origin, set);
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_string_array_condition(
	const ast::sf::string_array_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(condition.seq, symbols);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);

	std::vector<std::string> strings;
	for (auto& sobj : obj.values) {
		std::variant<lang::string, compile_error> str_or_err = detail::get_as<lang::string>(sobj);
		if (std::holds_alternative<compile_error>(str_or_err))
			return std::get<compile_error>(std::move(str_or_err));

		strings.push_back(std::move(std::get<lang::string>(str_or_err).value));
	}

	const auto condition_origin = parser::position_tag_of(condition);
	return add_string_array_condition(condition.property, strings, condition.exact_match.required, condition_origin, set);
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_has_influence_condition(
	const ast::sf::has_influence_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	namespace lim = lang::limits;

	std::variant<lang::object, compile_error> obj_or_err =
		detail::evaluate_sequence(condition.seq, symbols, 1, lim::max_filter_influences);

	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	influences_container influences;
	auto& obj = std::get<lang::object>(obj_or_err);
	for (auto& sobj : obj.values) {
		std::variant<lang::influence, compile_error> inf_or_err = detail::get_as<lang::influence>(sobj);
		if (std::holds_alternative<compile_error>(inf_or_err))
			return std::get<compile_error>(std::move(inf_or_err));

		influences.emplace_back(std::get<lang::influence>(inf_or_err));
	}

	return add_has_influence_condition(
		influences, condition.exact_match.required, parser::position_tag_of(condition), set.has_influence);
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_socket_spec_condition(
	const ast::sf::socket_spec_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(condition.seq, symbols, 1);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);

	lang::socket_spec_condition::container_type specs;
	for (auto& val : obj.values) {
		std::variant<lang::socket_spec, compile_error> ss_or_err = detail::get_as_socket_spec(val);
		if (std::holds_alternative<compile_error>(ss_or_err)) {
			return std::get<compile_error>(ss_or_err);
		}

		specs.push_back(std::get<lang::socket_spec>(ss_or_err));
	}

	return add_socket_spec_condition(
		condition.links_matter,
		lang::socket_spec_condition{
			condition.comparison_type,
			std::move(specs),
			parser::position_tag_of(condition)
		},
		set);
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_boolean_condition(
	const ast::sf::boolean_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& condition_set)
{
	std::variant<lang::object, compile_error> obj_or_err = detail::evaluate_sequence(condition.seq, symbols, 1, 1);
	if (std::holds_alternative<compile_error>(obj_or_err)) {
		return std::get<compile_error>(std::move(obj_or_err));
	}

	auto& obj = std::get<lang::object>(obj_or_err);
	BOOST_ASSERT(obj.values.size() == 1u);

	std::variant<lang::boolean, compile_error> bool_or_err = detail::get_as<lang::boolean>(obj.values[0]);
	if (std::holds_alternative<compile_error>(bool_or_err))
		return std::get<compile_error>(std::move(bool_or_err));

	return add_boolean_condition(
		std::get<lang::boolean>(bool_or_err),
		parser::position_tag_of(condition),
		condition.property,
		condition_set);
}

// ---- real filter helpers ----

[[nodiscard]] std::optional<compile_error>
real_filter_add_numeric_comparison_condition(
	ast::rf::numeric_condition numeric_condition,
	lang::condition_set& set)
{
	const lang::integer intgr = detail::evaluate(numeric_condition.integer);
	const auto origin = parser::position_tag_of(numeric_condition);
	const lang::comparison_type cmp = numeric_condition.comparison_type.value;
	return add_numeric_comparison_condition(numeric_condition.property, cmp, intgr, origin, set);
}

[[nodiscard]] std::optional<compile_error>
real_filter_add_string_array_condition(
	const parser::ast::rf::string_array_condition& condition,
	lang::condition_set& set)
{
	const auto origin = parser::position_tag_of(condition);
	const bool exact_match = condition.exact_match.required;

	std::vector<std::string> strings;
	strings.reserve(condition.string_literals.size());
	for (const auto& str : condition.string_literals) {
		strings.push_back(detail::evaluate(str).value);
	}

	return add_string_array_condition(condition.property, std::move(strings), exact_match, origin, set);
}

[[nodiscard]] std::optional<compile_error>
real_filter_add_has_influence_condition(
	const ast::rf::has_influence_condition& condition,
	std::optional<lang::influences_condition>& target)
{
	const auto num_inf = static_cast<int>(condition.influence_literals.size());
	const auto condition_origin = parser::position_tag_of(condition);

	namespace lim = lang::limits;
	if (num_inf < 1 || num_inf > lim::max_filter_influences) {
		return errors::invalid_amount_of_arguments{1, lim::max_filter_influences, num_inf, condition_origin};
	}

	influences_container influences;
	for (auto inf : condition.influence_literals) {
		influences.emplace_back(detail::evaluate(inf));
	}

	return add_has_influence_condition(influences, condition.exact_match.required, condition_origin, target);
}

[[nodiscard]] std::optional<compile_error>
real_filter_add_socket_spec_condition(
	const ast::rf::socket_spec_condition& condition,
	lang::condition_set& set)
{
	lang::socket_spec_condition::container_type specs;
	for (ast::rf::socket_spec_literal lit : condition.specs) {
		std::variant<lang::socket_spec, compile_error> ss_or_err =
			detail::evaluate_socket_spec_literal(lit.socket_count, lit.socket_colors);

		if (std::holds_alternative<compile_error>(ss_or_err)) {
			return std::get<compile_error>(std::move(ss_or_err));
		}

		specs.push_back(std::get<lang::socket_spec>(ss_or_err));
	}

	return add_socket_spec_condition(
		condition.links_matter,
		lang::socket_spec_condition{
			condition.comparison_type,
			std::move(specs),
			parser::position_tag_of(condition)
		},
		set);
}

} // namespace

namespace fs::compiler::detail
{

std::optional<compile_error>
spirit_filter_add_conditions(
	const std::vector<ast::sf::condition>& conditions,
	const lang::symbol_table& symbols,
	lang::spirit_condition_set& set)
{
	for (const ast::sf::condition& condition : conditions) {
		using result_type = std::optional<compile_error>;

		auto error = condition.apply_visitor(x3::make_lambda_visitor<result_type>(
			[&](const ast::sf::autogen_condition& autogen_condition) {
				return spirit_filter_add_autogen_condition(autogen_condition, set);
			},
			[&](const ast::sf::price_comparison_condition& comparison_condition) {
				return spirit_filter_add_price_comparison_condition(comparison_condition, symbols, set);
			},
			[&](const ast::sf::rarity_comparison_condition& comparison_condition) {
				return spirit_filter_add_rarity_comparison_condition(comparison_condition, symbols, set.conditions);
			},
			[&](const ast::sf::numeric_comparison_condition& comparison_condition) {
				return spirit_filter_add_numeric_comparison_condition(comparison_condition, symbols, set.conditions);
			},
			[&](const ast::sf::string_array_condition& string_condition) {
				return spirit_filter_add_string_array_condition(string_condition, symbols, set.conditions);
			},
			[&](const ast::sf::has_influence_condition& string_condition) {
				return spirit_filter_add_has_influence_condition(string_condition, symbols, set.conditions);
			},
			[&](const ast::sf::socket_spec_condition& socket_spec_condition) {
				return spirit_filter_add_socket_spec_condition(socket_spec_condition, symbols, set.conditions);
			},
			[&](const ast::sf::boolean_condition& boolean_condition) {
				return spirit_filter_add_boolean_condition(boolean_condition, symbols, set.conditions);
			}
		));

		if (error)
			return error;
	}

	return std::nullopt;
}

std::optional<compile_error>
real_filter_add_condition(
	const parser::ast::rf::condition& condition,
	lang::condition_set& condition_set)
{
	using result_type = std::optional<compile_error>;

	return condition.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](const ast::rf::rarity_condition& rarity_condition) -> result_type {
			return add_range_condition(
				rarity_condition.comparison_type.value,
				evaluate(rarity_condition.rarity),
				parser::position_tag_of(rarity_condition),
				condition_set.rarity);
		},
		[&](const ast::rf::numeric_condition& numeric_condition) -> result_type {
			return real_filter_add_numeric_comparison_condition(numeric_condition, condition_set);
		},
		[&](const ast::rf::string_array_condition& string_array_condition) -> result_type {
			return real_filter_add_string_array_condition(string_array_condition, condition_set);
		},
		[&](const ast::rf::has_influence_condition& has_influence_condition) -> result_type {
			return real_filter_add_has_influence_condition(has_influence_condition, condition_set.has_influence);
		},
		[&](const ast::rf::socket_spec_condition& ss_condition) -> result_type {
			return real_filter_add_socket_spec_condition(ss_condition, condition_set);
		},
		[&](const ast::rf::boolean_condition& boolean_condition) -> result_type {
			return add_boolean_condition(
				evaluate(boolean_condition.value),
				parser::position_tag_of(boolean_condition),
				boolean_condition.property,
				condition_set);
		}
	));
}

}
