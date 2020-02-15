#include <fs/compiler/detail/conditions.hpp>
#include <fs/compiler/detail/evaluate.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/condition_set.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <utility>

namespace ast = fs::parser::ast;

namespace x3 = boost::spirit::x3;

namespace
{

using namespace fs;
using namespace fs::compiler;

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

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_numeric_comparison_condition(
	const ast::sf::numeric_comparison_condition& condition,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set)
{
	// TODO implement

	const lang::position_tag condition_origin = parser::get_position_info(condition);

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::spirit_filter_add_numeric_comparison_condition,
		condition_origin
	};
}

[[nodiscard]] std::optional<compile_error>
add_string_condition_impl(
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
spirit_filter_add_influence_condition_impl(
	const std::vector<lang::influence>& influences,
	bool is_exact_match,
	lang::position_tag condition_origin,
	std::optional<lang::influences_condition>& target)
{
	if (target.has_value())
		return errors::condition_redefinition{condition_origin, (*target).origin};

	const auto contains = [&](lang::influence infl) {
		for (const auto in : influences)
			if (in == infl)
				return true;

		return false;
	};

	target = lang::influences_condition{
		contains(lang::influence::shaper),
		contains(lang::influence::elder),
		contains(lang::influence::crusader),
		contains(lang::influence::redeemer),
		contains(lang::influence::hunter),
		contains(lang::influence::warlord),
		is_exact_match,
		condition_origin
	};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
add_socket_group_condition(
	lang::socket_group socket_group,
	lang::position_tag condition_origin,
	std::optional<lang::socket_group_condition>& target)
{
	if (target.has_value())
		return errors::condition_redefinition{condition_origin, (*target).origin};

	target = lang::socket_group_condition{socket_group, condition_origin};
	return std::nullopt;
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_socket_spec_condition(
	const ast::sf::socket_spec_condition& condition,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set)
{
	// TODO implement
	throw 0;
	// return add_socket_group_condition(socket_group, condition_origin, condition_set.socket_group);
}

[[nodiscard]] std::optional<compile_error>
spirit_filter_add_boolean_condition(
	const ast::sf::boolean_condition& condition,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set)
{
	// TODO implement
	throw 0;
	// return add_boolean_condition(boolean, condition_origin, condition.property, condition_set);
}

[[nodiscard]] std::optional<compile_error>
real_filter_add_numeric_condition(
	ast::rf::numeric_condition numeric_condition,
	lang::condition_set& condition_set)
{
	const lang::integer integer = detail::evaluate(numeric_condition.integer);
	const auto origin = parser::get_position_info(numeric_condition);
	const lang::comparison_type cmp = numeric_condition.comparison_type.value;

	switch (numeric_condition.property) {
		case lang::numeric_comparison_condition_property::item_level: {
			return add_range_condition(cmp, integer.value, origin, condition_set.item_level);
		}
		case lang::numeric_comparison_condition_property::drop_level: {
			return add_range_condition(cmp, integer.value, origin, condition_set.drop_level);
		}
		case lang::numeric_comparison_condition_property::quality: {
			return add_range_condition(cmp, integer.value, origin, condition_set.quality);
		}
		case lang::numeric_comparison_condition_property::sockets: {
			return add_range_condition(cmp, integer.value, origin, condition_set.sockets);
		}
		case lang::numeric_comparison_condition_property::links: {
			return add_range_condition(cmp, integer.value, origin, condition_set.links);
		}
		case lang::numeric_comparison_condition_property::height: {
			return add_range_condition(cmp, integer.value, origin, condition_set.height);
		}
		case lang::numeric_comparison_condition_property::width: {
			return add_range_condition(cmp, integer.value, origin, condition_set.width);
		}
		case lang::numeric_comparison_condition_property::stack_size: {
			return add_range_condition(cmp, integer.value, origin, condition_set.stack_size);
		}
		case lang::numeric_comparison_condition_property::gem_level: {
			return add_range_condition(cmp, integer.value, origin, condition_set.gem_level);
		}
		case lang::numeric_comparison_condition_property::map_tier: {
			return add_range_condition(cmp, integer.value, origin, condition_set.map_tier);
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::real_filter_add_numeric_condition,
		origin
	};
}

[[nodiscard]] std::optional<compile_error>
real_filter_add_string_array_condition(
	const parser::ast::rf::string_array_condition& condition,
	lang::condition_set& condition_set)
{
	const auto origin = parser::get_position_info(condition);
	const bool exact_match = condition.exact_match.required;

	std::vector<std::string> strings;
	strings.reserve(condition.string_literals.size());
	for (const auto& str : condition.string_literals) {
		strings.push_back(detail::evaluate(str).value);
	}

	switch (condition.property) {
		case lang::string_array_condition_property::class_: {
			return add_string_condition_impl(std::move(strings), exact_match, origin, condition_set.class_);
		}
		case lang::string_array_condition_property::base_type: {
			return add_string_condition_impl(std::move(strings), exact_match, origin, condition_set.base_type);
		}
		case lang::string_array_condition_property::has_explicit_mod: {
			return add_string_condition_impl(std::move(strings), exact_match, origin, condition_set.has_explicit_mod);
		}
		case lang::string_array_condition_property::has_enchantment: {
			return add_string_condition_impl(std::move(strings), exact_match, origin, condition_set.has_enchantment);
		}
		case lang::string_array_condition_property::prophecy: {
			return add_string_condition_impl(std::move(strings), exact_match, origin, condition_set.prophecy);
		}
	}

	return errors::internal_compiler_error{
		errors::internal_compiler_error_cause::real_filter_add_string_array_condition,
		origin
	};
}

[[nodiscard]] std::optional<compile_error>
real_filter_add_has_influence_condition(
	const ast::rf::has_influence_condition& condition,
	std::optional<lang::influences_condition>& target)
{
	const auto origin = parser::get_position_info(condition);

	if (target.has_value())
		return errors::condition_redefinition{origin, origin};

	const auto count = [&](lang::influence i) {
		int result = 0;

		for (parser::ast::rf::influence_literal il : condition.influence_literals) {
			if (detail::evaluate(il) == i)
				++result;
		}

		return result;
	};

	// can add an extra error handling here later
	const auto count_shaper   = count(lang::influence::shaper);
	const auto count_elder    = count(lang::influence::elder);
	const auto count_crusader = count(lang::influence::crusader);
	const auto count_redeemer = count(lang::influence::redeemer);
	const auto count_hunter   = count(lang::influence::hunter);
	const auto count_warlord  = count(lang::influence::warlord);

	target = lang::influences_condition{
		count_shaper   > 0,
		count_elder    > 0,
		count_crusader > 0,
		count_redeemer > 0,
		count_hunter   > 0,
		count_warlord  > 0,
		condition.exact_match.required,
		origin
	};
	return std::nullopt;
}

} // namespace

namespace fs::compiler::detail
{

std::optional<compile_error>
spirit_filter_add_conditions(
	const std::vector<ast::sf::condition>& conditions,
	const lang::symbol_table& symbols,
	const lang::item_price_data& item_price_data,
	lang::condition_set& condition_set)
{
	for (const ast::sf::condition& condition : conditions) {
		using result_type = std::optional<compile_error>;

		auto error = condition.apply_visitor(x3::make_lambda_visitor<result_type>(
			[&](const ast::sf::rarity_comparison_condition& comparison_condition) -> result_type {
				throw 0; // TODO implement
			},
			[&](const ast::sf::numeric_comparison_condition& comparison_condition) {
				return spirit_filter_add_numeric_comparison_condition(comparison_condition, symbols, item_price_data, condition_set);
			},
			[&](const ast::sf::string_sequence_condition& string_condition) -> result_type {
				throw 0; // TODO implement
			},
			[&](const ast::sf::has_influence_condition& string_condition) -> result_type {
				throw 0; // TODO implement
			},
			[&](const ast::sf::socket_spec_condition& socket_group_condition) {
				return spirit_filter_add_socket_spec_condition(socket_group_condition, symbols, item_price_data, condition_set);
			},
			[&](const ast::sf::boolean_condition& boolean_condition) {
				return spirit_filter_add_boolean_condition(boolean_condition, symbols, item_price_data, condition_set);
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
				parser::get_position_info(rarity_condition),
				condition_set.rarity);
		},
		[&](const ast::rf::numeric_condition& numeric_condition) -> result_type {
			return real_filter_add_numeric_condition(numeric_condition, condition_set);
		},
		[&](const ast::rf::string_array_condition& string_array_condition) -> result_type {
			return real_filter_add_string_array_condition(string_array_condition, condition_set);
		},
		[&](const ast::rf::has_influence_condition& has_influence_condition) -> result_type {
			return real_filter_add_has_influence_condition(has_influence_condition, condition_set.has_influence);
		},
		[&](const ast::rf::socket_group_condition& sg_condition) -> result_type {
			const auto origin = parser::get_position_info(sg_condition);
			std::variant<lang::socket_group, compile_error> sg_or_error =
				evaluate_as_socket_group(sg_condition.group.value, origin);

			if (std::holds_alternative<compile_error>(sg_or_error))
				return std::get<compile_error>(std::move(sg_or_error));

			return add_socket_group_condition(
				std::get<lang::socket_group>(sg_or_error),
				origin,
				condition_set.socket_group);
		},
		[&](const ast::rf::boolean_condition& boolean_condition) -> result_type {
			return add_boolean_condition(
				evaluate(boolean_condition.value),
				parser::get_position_info(boolean_condition),
				boolean_condition.property,
				condition_set);
		}
	));
}

}
