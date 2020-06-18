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
[[nodiscard]] outcome<>
add_range_condition(
	lang::comparison_type comparison_type,
	T value,
	lang::position_tag condition_origin,
	lang::range_condition<T>& target)
{
	switch (comparison_type) {
		case lang::comparison_type::equal: {
			if (target.lower_bound.has_value())
				return error(errors::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin});

			if (target.upper_bound.has_value())
				return error(errors::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin});

			target.set_exact(value, condition_origin);
			return outcome<>::success();
		}
		case lang::comparison_type::less: {
			if (target.upper_bound.has_value())
				return error(errors::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin});

			target.set_upper_bound(value, false, condition_origin);
			return outcome<>::success();
		}
		case lang::comparison_type::less_equal: {
			if (target.upper_bound.has_value())
				return error(errors::upper_bound_redefinition{condition_origin, (*target.upper_bound).origin});

			target.set_upper_bound(value, true, condition_origin);
			return outcome<>::success();
		}
		case lang::comparison_type::greater: {
			if (target.lower_bound.has_value())
				return error(errors::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin});

			target.set_lower_bound(value, false, condition_origin);
			return outcome<>::success();
		}
		case lang::comparison_type::greater_equal: {
			if (target.lower_bound.has_value())
				return error(errors::lower_bound_redefinition{condition_origin, (*target.lower_bound).origin});

			target.set_lower_bound(value, true, condition_origin);
			return outcome<>::success();
		}
	}

	return error(errors::internal_compiler_error{
		errors::internal_compiler_error_cause::add_range_condition,
		condition_origin
	});
}

[[nodiscard]] outcome<>
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

	return error(errors::internal_compiler_error{
		errors::internal_compiler_error_cause::add_numeric_comparison_condition,
		condition_origin
	});
}

[[nodiscard]] outcome<>
add_string_array_condition_impl(
	std::vector<std::string> strings,
	bool is_exact_match,
	lang::position_tag condition_origin,
	std::optional<lang::strings_condition>& target)
{
	if (target.has_value())
		return error(errors::condition_redefinition{condition_origin, (*target).origin});

	target = lang::strings_condition{std::move(strings), is_exact_match, condition_origin};
	return outcome<>::success();
}

[[nodiscard]] outcome<>
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

	return error(errors::internal_compiler_error{
		errors::internal_compiler_error_cause::add_string_array_condition,
		condition_origin
	});
}

/*
 * Empty container represents "HasInfluence None".
 *
 * This is a bit limiting implementation, because
 * it does not allow to track all possible origins
 * (especially "None" literal origin) but such detailed
 * debug information is not currently needed; also many
 * other conditions also track only condition name origin.
*/
using influences_container = boost::container::static_vector<
	lang::influence,
	lang::limits::max_filter_influences
>;

[[nodiscard]] outcome<>
add_has_influence_condition(
	settings st,
	influences_container influences,
	bool is_exact_match,
	lang::position_tag origin,
	std::optional<lang::influences_condition>& target)
{
	log_container logs;
	for (std::size_t i = 0; i < influences.size(); ++i) {
		for (std::size_t j = 0; j < influences.size(); ++j) {
			if (j != i && influences[i] == influences[j]) {
				logs.emplace_back(error(errors::duplicate_influence{
					influences[i].origin, influences[j].origin}));

				if (!should_continue(st.error_handling, logs))
					return logs;
			}
		}
	}

	if (target.has_value())
		logs.emplace_back(error(errors::condition_redefinition{origin, (*target).origin}));

	if (!should_continue(st.error_handling, logs))
		return logs;

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
	return outcome<>::success(std::move(logs));
}

[[nodiscard]] outcome<>
add_socket_spec_condition(
	bool links_matter,
	lang::socket_spec_condition condition,
	lang::condition_set& set)
{
	auto& target = links_matter ? set.socket_group : set.sockets;

	if (target.has_value())
		return error(errors::condition_redefinition{condition.origin, (*target).origin});

	target = std::move(condition);
	return outcome<>::success();
}

[[nodiscard]] outcome<>
add_boolean_condition_impl(
	lang::boolean boolean,
	lang::position_tag condition_origin,
	std::optional<lang::boolean_condition>& target)
{
	if (target.has_value())
		return error(errors::condition_redefinition{condition_origin, (*target).origin});

	target = lang::boolean_condition{boolean, condition_origin};
	return outcome<>::success();
}

[[nodiscard]] outcome<>
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

	return error(errors::internal_compiler_error{
		errors::internal_compiler_error_cause::add_boolean_condition,
		condition_origin
	});
}

// ---- spirit filter helpers ----

[[nodiscard]] outcome<>
spirit_filter_add_autogen_condition(
	const ast::sf::autogen_condition& condition,
	lang::spirit_condition_set& set)
{
	const auto condition_origin = parser::position_tag_of(condition);

	if (set.autogen.has_value())
		return error(errors::condition_redefinition{condition_origin, (*set.autogen).origin});

	set.autogen = lang::autogen_condition{condition.cat_expr.category, condition_origin};
	return outcome<>::success();
}

[[nodiscard]] outcome<>
spirit_filter_add_price_comparison_condition(
	settings st,
	const ast::sf::price_comparison_condition& condition,
	const lang::symbol_table& symbols,
	lang::spirit_condition_set& set)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, 1)
		.map_result<lang::fractional>([](lang::object obj) {
			BOOST_ASSERT(obj.values.size() == 1u);
			return detail::get_as_fractional(obj.values[0]);
		})
		.map_result([&](lang::fractional frac) {
			return add_range_condition(
				condition.comparison_type.value,
				frac,
				parser::position_tag_of(condition),
				set.price);
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_rarity_comparison_condition(
	settings st,
	const ast::sf::rarity_comparison_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, 1)
		.map_result<lang::rarity>([](lang::object obj) {
			BOOST_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::rarity>(obj.values[0]);
		})
		.map_result([&](lang::rarity rar) {
			return add_range_condition(
				condition.comparison_type.value,
				rar,
				parser::position_tag_of(condition),
				set.rarity);
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_numeric_comparison_condition(
	settings st,
	const ast::sf::numeric_comparison_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, 1)
		.map_result<lang::integer>([](lang::object obj) {
			BOOST_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::integer>(obj.values[0]);
		})
		.map_result([&](lang::integer intgr) {
			const auto origin = parser::position_tag_of(condition);
			return add_numeric_comparison_condition(
				condition.property, condition.comparison_type.value, intgr, origin, set);
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_string_array_condition(
	settings st,
	const ast::sf::string_array_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	return detail::evaluate_sequence(st, condition.seq, symbols)
		.map_result<std::vector<std::string>>([&](lang::object obj) -> outcome<std::vector<std::string>> {
			std::vector<std::string> strings;
			log_container logs;

			for (auto& sobj : obj.values) {
				detail::get_as<lang::string>(sobj)
					.map_result([&](lang::string str) { strings.push_back(std::move(str.value)); })
					.move_logs_to(logs);

				if (!should_continue(st.error_handling, logs))
					return logs;
			}

			return {std::move(strings), std::move(logs)};
		})
		.map_result([&](std::vector<std::string> strings) {
			return add_string_array_condition(
				condition.property,
				std::move(strings),
				condition.exact_match.required,
				parser::position_tag_of(condition),
				set);
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_has_influence_condition(
	settings st,
	const ast::sf::has_influence_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, lang::limits::max_filter_influences)
		.map_result<influences_container>([&](lang::object obj) -> outcome<influences_container> {
			if (obj.values.size() == 1u) {
				auto none = detail::get_as<lang::none>(obj.values.front());
				if (none.has_result())
					return influences_container();
			}

			influences_container influences;
			log_container logs;

			for (auto& sobj : obj.values) {
				detail::get_as<lang::influence>(sobj)
					.map_result([&](lang::influence inf) { influences.emplace_back(inf); })
					.move_logs_to(logs);

				if (!should_continue(st.error_handling, logs))
					return logs;
			}

			return {std::move(influences), std::move(logs)};
		})
		.map_result([&](influences_container influences) {
			return add_has_influence_condition(
				st,
				std::move(influences),
				condition.exact_match.required,
				parser::position_tag_of(condition),
				set.has_influence);
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_socket_spec_condition(
	settings st,
	const ast::sf::socket_spec_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& set)
{
	using specs_t = lang::socket_spec_condition::container_type;

	return detail::evaluate_sequence(st, condition.seq, symbols, 1)
		.map_result<specs_t>([&](lang::object obj) -> outcome<specs_t> {
			specs_t specs;
			log_container logs;

			for (auto& val : obj.values) {
				detail::get_as_socket_spec(val)
					.map_result([&](lang::socket_spec ss) { specs.push_back(ss); })
					.move_logs_to(logs);


				if (!should_continue(st.error_handling, logs))
					return logs;
			}

			return {std::move(specs), std::move(logs)};
		})
		.map_result([&](specs_t specs) {
			return add_socket_spec_condition(
				condition.links_matter,
				lang::socket_spec_condition{
					condition.comparison_type,
					std::move(specs),
					parser::position_tag_of(condition)
				},
				set);
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_boolean_condition(
	settings st,
	const ast::sf::boolean_condition& condition,
	const lang::symbol_table& symbols,
	lang::condition_set& condition_set)
{
	return detail::evaluate_sequence(st, condition.seq, symbols, 1, 1)
		.map_result<lang::boolean>([](lang::object obj) {
			BOOST_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::boolean>(obj.values[0]);
		})
		.map_result([&](lang::boolean b) {
			return add_boolean_condition(
				b,
				parser::position_tag_of(condition),
				condition.property,
				condition_set);
		});
}

// ---- real filter helpers ----

[[nodiscard]] outcome<>
real_filter_add_numeric_comparison_condition(
	ast::rf::numeric_condition numeric_condition,
	lang::condition_set& set)
{
	const lang::integer intgr = detail::evaluate(numeric_condition.integer);
	const auto origin = parser::position_tag_of(numeric_condition);
	const lang::comparison_type cmp = numeric_condition.comparison_type.value;
	return add_numeric_comparison_condition(numeric_condition.property, cmp, intgr, origin, set);
}

[[nodiscard]] outcome<>
real_filter_add_string_array_condition(
	const parser::ast::rf::string_array_condition& condition,
	lang::condition_set& set)
{
	std::vector<std::string> strings;
	strings.reserve(condition.string_literals.size());
	for (const auto& str : condition.string_literals) {
		strings.push_back(detail::evaluate(str).value);
	}

	const auto origin = parser::position_tag_of(condition);
	const bool exact_match = condition.exact_match.required;
	return add_string_array_condition(condition.property, std::move(strings), exact_match, origin, set);
}

[[nodiscard]] outcome<influences_container>
real_filter_make_influences_container(
	const ast::rf::influence_spec& spec,
	lang::position_tag condition_origin)
{
	using result_type = outcome<influences_container>;

	return spec.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](const ast::rf::influence_literal_array& array) -> result_type {
			const auto num_inf = static_cast<int>(array.size());

			if (num_inf < 1 || num_inf > lang::limits::max_filter_influences) {
				return error(errors::invalid_amount_of_arguments{
					/* min */ 1, /* max */ lang::limits::max_filter_influences, /* actual */ num_inf, condition_origin});
			}

			influences_container influences;
			for (auto inf : array) {
				influences.emplace_back(detail::evaluate(inf));
			}

			return influences;
		},
		[](ast::rf::none_literal /* literal */) -> result_type {
			return influences_container();
		}
	));
}

[[nodiscard]] outcome<>
real_filter_add_has_influence_condition(
	settings st,
	const ast::rf::has_influence_condition& condition,
	std::optional<lang::influences_condition>& target)
{
	const auto condition_origin = parser::position_tag_of(condition);

	return real_filter_make_influences_container(condition.spec, condition_origin)
		.map_result([&](influences_container influences) {
			return add_has_influence_condition(
				st,
				std::move(influences),
				condition.exact_match.required,
				condition_origin,
				target);
		});

}

[[nodiscard]] outcome<>
real_filter_add_socket_spec_condition(
	settings st,
	const ast::rf::socket_spec_condition& condition,
	lang::condition_set& set)
{
	lang::socket_spec_condition::container_type specs;
	log_container logs;

	for (ast::rf::socket_spec_literal lit : condition.specs) {
		detail::evaluate_socket_spec_literal(st, lit.socket_count, lit.socket_colors)
			.map_result([&](lang::socket_spec ss) { specs.push_back(ss); })
			.move_logs_to(logs);

		if (!should_continue(st.error_handling, logs))
			return logs;
	}

	return outcome<>::success(std::move(logs))
		.merge_with(add_socket_spec_condition(
			condition.links_matter,
			lang::socket_spec_condition{
				condition.comparison_type,
				std::move(specs),
				parser::position_tag_of(condition)
			},
			set));
}

} // namespace

namespace fs::compiler::detail
{

outcome<>
spirit_filter_add_conditions(
	settings st,
	const std::vector<ast::sf::condition>& conditions,
	const lang::symbol_table& symbols,
	lang::spirit_condition_set& set)
{
	log_container logs;
	int failed_conditions = 0;

	for (const ast::sf::condition& condition : conditions) {
		auto condition_outcome = condition.apply_visitor(x3::make_lambda_visitor<outcome<>>(
			[&](const ast::sf::autogen_condition& cond) {
				return spirit_filter_add_autogen_condition(cond, set);
			},
			[&](const ast::sf::price_comparison_condition& cond) {
				return spirit_filter_add_price_comparison_condition(st, cond, symbols, set);
			},
			[&](const ast::sf::rarity_comparison_condition& cond) {
				return spirit_filter_add_rarity_comparison_condition(st, cond, symbols, set.conditions);
			},
			[&](const ast::sf::numeric_comparison_condition& cond) {
				return spirit_filter_add_numeric_comparison_condition(st, cond, symbols, set.conditions);
			},
			[&](const ast::sf::string_array_condition& cond) {
				return spirit_filter_add_string_array_condition(st, cond, symbols, set.conditions);
			},
			[&](const ast::sf::has_influence_condition& cond) {
				return spirit_filter_add_has_influence_condition(st, cond, symbols, set.conditions);
			},
			[&](const ast::sf::socket_spec_condition& cond) {
				return spirit_filter_add_socket_spec_condition(st, cond, symbols, set.conditions);
			},
			[&](const ast::sf::boolean_condition& cond) {
				return spirit_filter_add_boolean_condition(st, cond, symbols, set.conditions);
			}
		));

		if (!condition_outcome.has_result())
			++failed_conditions;

		if (!should_continue(st.error_handling, condition_outcome)) {
			condition_outcome.move_logs_to(logs);
			logs.emplace_back(note{notes::failed_operations_count{"condition", failed_conditions}});
			return logs;
		}

		condition_outcome.move_logs_to(logs);
	}

	if (failed_conditions > 0)
		logs.emplace_back(note{notes::failed_operations_count{"condition", failed_conditions}});

	return outcome<>::success(std::move(logs));
}

outcome<>
real_filter_add_condition(
	settings st,
	const parser::ast::rf::condition& condition,
	lang::condition_set& condition_set)
{
	return condition.apply_visitor(x3::make_lambda_visitor<outcome<>>(
		[&](const ast::rf::rarity_condition& cond) {
			return add_range_condition(
				cond.comparison_type.value,
				evaluate(cond.rarity),
				parser::position_tag_of(cond),
				condition_set.rarity);
		},
		[&](const ast::rf::numeric_condition& cond) {
			return real_filter_add_numeric_comparison_condition(cond, condition_set);
		},
		[&](const ast::rf::string_array_condition& cond) {
			return real_filter_add_string_array_condition(cond, condition_set);
		},
		[&](const ast::rf::has_influence_condition& cond) {
			return real_filter_add_has_influence_condition(st, cond, condition_set.has_influence);
		},
		[&](const ast::rf::socket_spec_condition& cond) {
			return real_filter_add_socket_spec_condition(st, cond, condition_set);
		},
		[&](const ast::rf::boolean_condition& cond) {
			return add_boolean_condition(
				evaluate(cond.value),
				parser::position_tag_of(cond),
				cond.property,
				condition_set);
		}
	));
}

}
