#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/lang/limits.hpp>
#include <fs/utility/assert.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>

#include <utility>
#include <type_traits>

namespace ast = fs::parser::ast;
namespace x3 = boost::spirit::x3;

namespace
{

using namespace fs;
using namespace fs::compiler;

// ---- spirit filter helpers ----

[[nodiscard]] outcome<>
spirit_filter_add_set_color_action_impl(
	lang::color_action_type action_type,
	lang::color_action action,
	lang::action_set& set)
{
	// override previous action
	switch (action_type) {
		case lang::color_action_type::set_border_color: {
			set.set_border_color = action;
			return outcome<>::success();
		}
		case lang::color_action_type::set_text_color: {
			set.set_text_color = action;
			return outcome<>::success();
		}
		case lang::color_action_type::set_background_color: {
			set.set_background_color = action;
			return outcome<>::success();
		}
	}

	return error(errors::internal_compiler_error{
		errors::internal_compiler_error_cause::spirit_filter_add_set_color_action_impl,
		action.origin});
}

[[nodiscard]] outcome<>
spirit_filter_add_set_color_action(
	settings st,
	const ast::sf::set_color_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 3, 4)
		.map_result<lang::color>([&](lang::object obj) {
			const auto num_values = obj.values.size();
			BOOST_ASSERT(num_values == 3u || num_values == 4u);

			auto rgb = detail::get_as<lang::integer>(obj.values[0])
				.merge_with(detail::get_as<lang::integer>(obj.values[1]))
				.merge_with(detail::get_as<lang::integer>(obj.values[2]));

			if (num_values == 4u) {
				return std::move(rgb)
					.merge_with(detail::get_as<lang::integer>(obj.values[3]))
					.map_result<lang::color>(
						[&](lang::integer r, lang::integer g, lang::integer b, lang::integer a) {
							return detail::make_color(st, r, g, b, a);
						}
					);
			}
			else {
				return std::move(rgb)
					.map_result<lang::color>(
						[&](lang::integer r, lang::integer g, lang::integer b) {
							return detail::make_color(st, r, g, b);
						}
					);
			}
		})
		.map_result([&](lang::color col) {
			auto act = lang::color_action{col, parser::position_tag_of(action)};
			return spirit_filter_add_set_color_action_impl(action.action_type, act, set);
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_set_font_size_action(
	settings st,
	const ast::sf::set_font_size_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 1)
		.map_result<lang::integer>([](lang::object obj) {
			BOOST_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::integer>(obj.values[0]);
		})
		.map_result<lang::font_size_action>([&](lang::integer font_size) {
			outcome<lang::font_size_action> result(lang::font_size_action{font_size, parser::position_tag_of(action)});

			if (font_size.value < lang::limits::min_filter_font_size
				|| font_size.value > lang::limits::max_filter_font_size)
			{
				result.logs().emplace_back(warning(warnings::font_size_outside_range{font_size.origin}));
			}

			return result;
		})
		.map_result([&](lang::font_size_action fsa) {
			set.set_font_size = fsa;
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_minimap_icon_action(
	settings st,
	const ast::sf::minimap_icon_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 3, 3)
		.map_result<lang::minimap_icon>([](lang::object obj) {
			BOOST_ASSERT(obj.values.size() == 3u);

			return detail::get_as<lang::integer>(obj.values[0])
				.merge_with(detail::get_as<lang::suit>(obj.values[1]))
				.merge_with(detail::get_as<lang::shape>(obj.values[2]))
				.map_result<lang::minimap_icon>(detail::make_minimap_icon);
		})
		.map_result([&](lang::minimap_icon icon) {
			// override previous action
			set.minimap_icon = lang::minimap_icon_action{icon, parser::position_tag_of(action)};
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_play_effect_action(
	settings st,
	const ast::sf::play_effect_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 2)
		.map_result<lang::play_effect>([](lang::object obj) {
			const auto num_values = obj.values.size();
			BOOST_ASSERT(num_values == 1u || num_values == 2u);

			return detail::get_as<lang::suit>(obj.values[0])
				.merge_with([&]() -> outcome<bool> {
					if (num_values != 2)
						return false; // no "Temp" token, therefore false

					BOOST_ASSERT(num_values == 2u);

					// there is an extra token, it must be "Temp"; if successful map to true
					return detail::get_as<lang::temp>(obj.values[1])
						.map_result<bool>([](lang::temp) { return true; });
				}())
				.map_result<lang::play_effect>([](lang::suit s, bool is_temp) {
					return lang::play_effect{s, is_temp};
				});
		})
		.map_result([&](lang::play_effect effect) {
			// override previous action
			set.play_effect = lang::play_effect_action{effect, parser::position_tag_of(action)};
		});
}

[[nodiscard]] outcome<lang::builtin_alert_sound_id>
make_builtin_alert_sound_id(
	const lang::single_object& sobj)
{
	auto integer_sound_id = detail::get_as<lang::integer>(sobj);
	auto shaper_sound_id = detail::get_as<lang::shaper_voice_line>(sobj);

	if (integer_sound_id.has_result()) {
		return std::move(integer_sound_id).map_result<lang::builtin_alert_sound_id>([](lang::integer sound_id) {
			return detail::make_builtin_alert_sound_id(sound_id);
		});
	}
	else if (shaper_sound_id.has_result()) {
		return std::move(shaper_sound_id).map_result<lang::builtin_alert_sound_id>([](lang::shaper_voice_line sound_id) {
			return detail::make_builtin_alert_sound_id(sound_id);
		});
	}
	else {
		log_container logs;
		// main error
		logs.emplace_back(error(errors::invalid_builtin_alert_sound_id{sobj.origin}));
		// errors specifying each sound ID variant problems
		logs.emplace_back(note{notes::fixed_text{"in attempt of integer sound ID"}});
		integer_sound_id.move_logs_to(logs);
		logs.emplace_back(note{notes::fixed_text{"in attempt of Shaper voice line sound ID"}});
		shaper_sound_id.move_logs_to(logs);
		return logs;
	}
}

[[nodiscard]] outcome<lang::builtin_alert_sound>
make_builtin_alert_sound(
	settings st,
	bool positional,
	const lang::object& obj)
{
	const auto num_args = obj.values.size();
	BOOST_ASSERT(num_args == 1u || num_args == 2u);

	auto sound_id = make_builtin_alert_sound_id(obj.values[0]);

	if (num_args == 2u) {
		return std::move(sound_id)
			.merge_with(detail::get_as<lang::integer>(obj.values[1]))
			.map_result<lang::builtin_alert_sound>([&](lang::builtin_alert_sound_id sound_id, lang::integer volume) {
				return detail::make_builtin_alert_sound(st, positional, sound_id, volume);
			});
	}
	else {
		return std::move(sound_id)
			.map_result<lang::builtin_alert_sound>([&](lang::builtin_alert_sound_id sound_id) {
				return detail::make_builtin_alert_sound(st, positional, sound_id);
			});
	}
}

[[nodiscard]] outcome<>
spirit_filter_add_play_alert_sound_action(
	settings st,
	const ast::sf::play_alert_sound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 2)
		.map_result<lang::builtin_alert_sound>([&](lang::object obj) {
			BOOST_ASSERT(obj.values.size() == 1u || obj.values.size() == 2u);
			return make_builtin_alert_sound(st, action.positional, obj);
		})
		.map_result([&](lang::builtin_alert_sound bas) {
			// override previous action
			set.play_alert_sound = lang::alert_sound_action{
				lang::alert_sound{bas},
				parser::position_tag_of(action)
			};
		});
}

[[nodiscard]] outcome<lang::custom_alert_sound>
make_custom_alert_sound(
	settings /* st */,
	const lang::single_object& sobj)
{
	return detail::get_as<lang::string>(sobj)
		.map_result<lang::custom_alert_sound>([](lang::string str) {
			return lang::custom_alert_sound{std::move(str)};
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_custom_alert_sound_action(
	settings st,
	const ast::sf::custom_alert_sound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 1)
		.map_result<lang::custom_alert_sound>([&](lang::object obj) {
			BOOST_ASSERT(obj.values.size() == 1u);
			return make_custom_alert_sound(st, obj.values[0]);
		})
		.map_result([&](lang::custom_alert_sound cas) {
			set.play_alert_sound = lang::alert_sound_action{
				lang::alert_sound{std::move(cas)},
				parser::position_tag_of(action)
			};
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_set_alert_sound_action(
	settings st,
	const ast::sf::set_alert_sound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 2)
		.map_result<lang::alert_sound>([&](lang::object obj) -> outcome<lang::alert_sound> {
			const auto num_values = obj.values.size();
			BOOST_ASSERT(num_values == 1u || num_values == 2u);

			// SetAlertSound always generates non-positional sounds, hence the false
			auto builtin_alert = make_builtin_alert_sound(st, false, obj);
			auto custom_alert = [&]() -> outcome<lang::custom_alert_sound> {
				if (num_values == 2u) {
					return error(errors::invalid_amount_of_arguments{
						/* min */ 1, /* max */ 1, /* actual */ 2, parser::position_tag_of(action.seq)
					});
				}

				BOOST_ASSERT(num_values == 1u);
				return make_custom_alert_sound(st, obj.values[0]);
			}();

			if (custom_alert.has_result()) {
				return std::move(custom_alert)
					.map_result<lang::alert_sound>([](lang::custom_alert_sound cas) {
						return lang::alert_sound{std::move(cas)};
					});
			}
			else if (builtin_alert.has_result()) {
				return std::move(builtin_alert)
					.map_result<lang::alert_sound>([](lang::builtin_alert_sound bas) {
						return lang::alert_sound{std::move(bas)};
					});
			}
			else {
				log_container logs;
				// main error
				logs.emplace_back(error(
					errors::invalid_set_alert_sound{parser::position_tag_of(action.seq)}));
				// errors specifying each sound variant problems
				logs.emplace_back(note{notes::fixed_text{"in attempt of custom alert sound"}});
				custom_alert.move_logs_to(logs);
				logs.emplace_back(note{notes::fixed_text{"in attempt of built-in alert sound"}});
				builtin_alert.move_logs_to(logs);
				return logs;
			}
		})
		.map_result([&](lang::alert_sound alert) {
			set.play_alert_sound = lang::alert_sound_action{
				lang::alert_sound{std::move(alert)},
				parser::position_tag_of(action)
			};
		});
}

[[nodiscard]] outcome<>
spirit_filter_add_compound_action(
	settings st,
	const ast::sf::compound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 1)
		.map_result<lang::action_set>([](lang::object obj) {
			BOOST_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::action_set>(obj.values[0]);
		})
		.map_result([&](lang::action_set as) {
			set.override_with(std::move(as));
		});
}

// ---- real filter helpers ----

template <typename T>
[[nodiscard]] outcome<>
real_filter_add_action_impl(
	T&& action,
	std::optional<T>& target)
{
	if (target) {
		return error(errors::action_redefinition{action.origin, (*target).origin});
	}

	target = std::move(action);
	return outcome<>::success();
}

[[nodiscard]] outcome<>
real_filter_add_color_action(
	settings st,
	const ast::rf::color_action& action,
	lang::action_set& set)
{
	return detail::evaluate(st, action.color)
		.map_result([&](lang::color col) -> outcome<> {
			auto act = lang::color_action{col, parser::position_tag_of(action)};

			switch (action.action) {
				case lang::color_action_type::set_border_color: {
					return real_filter_add_action_impl(std::move(act), set.set_border_color);
				}
				case lang::color_action_type::set_text_color: {
					return real_filter_add_action_impl(std::move(act), set.set_text_color);
				}
				case lang::color_action_type::set_background_color: {
					return real_filter_add_action_impl(std::move(act), set.set_background_color);
				}
			}

			return error(errors::internal_compiler_error{
				errors::internal_compiler_error_cause::real_filter_add_color_action,
				act.origin});
		});
}

[[nodiscard]] lang::builtin_alert_sound_id
real_filter_make_builtin_alert_sound_id(
	ast::rf::sound_id sound_id)
{
	return sound_id.apply_visitor(x3::make_lambda_visitor<lang::builtin_alert_sound_id>(
		[](ast::rf::integer_literal sound_id) {
			return lang::builtin_alert_sound_id{detail::evaluate(sound_id)};
		},
		[](ast::rf::shaper_voice_line_literal sound_id) {
			return lang::builtin_alert_sound_id{detail::evaluate(sound_id)};
		}
	));
}

[[nodiscard]] lang::alert_sound
real_filter_make_builtin_alert_sound(
	ast::rf::sound_id sound_id,
	boost::optional<ast::rf::integer_literal> volume,
	bool positional)
{
	if (volume) {
		return lang::alert_sound{lang::builtin_alert_sound{
			positional,
			real_filter_make_builtin_alert_sound_id(sound_id),
			detail::evaluate(*volume)
		}};
	}
	else {
		return lang::alert_sound{lang::builtin_alert_sound{
			positional,
			real_filter_make_builtin_alert_sound_id(sound_id)
		}};
	}
}

} // namespace

namespace fs::compiler::detail
{

outcome<>
spirit_filter_add_action(
	settings st,
	const ast::sf::action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set)
{
	return action.apply_visitor(x3::make_lambda_visitor<outcome<>>(
		[&](const ast::sf::set_color_action& a) {
			return spirit_filter_add_set_color_action(st, a, symbols, set);
		},
		[&](const ast::sf::set_font_size_action& a) {
			return spirit_filter_add_set_font_size_action(st, a, symbols, set);
		},
		[&](const ast::sf::minimap_icon_action& a) {
			return spirit_filter_add_minimap_icon_action(st, a, symbols, set);
		},
		[&](const ast::sf::play_effect_action& a) {
			return spirit_filter_add_play_effect_action(st, a, symbols, set);
		},
		[&](const ast::sf::play_alert_sound_action& a) {
			return spirit_filter_add_play_alert_sound_action(st, a, symbols, set);
		},
		[&](const ast::sf::custom_alert_sound_action& a) {
			return spirit_filter_add_custom_alert_sound_action(st, a, symbols, set);
		},
		[&](const ast::sf::set_alert_sound_action& a) {
			return spirit_filter_add_set_alert_sound_action(st, a, symbols, set);
		},
		[&](const ast::sf::disable_drop_sound_action&) {
			set.disable_drop_sound = lang::disable_drop_sound_action{parser::position_tag_of(action)};
			return outcome<>::success();
		},
		[&](const ast::sf::compound_action& ca) {
			return spirit_filter_add_compound_action(st, ca, symbols, set);
		}
	));
}

outcome<>
real_filter_add_action(
	settings st,
	const parser::ast::rf::action& a,
	lang::action_set& set)
{
	using result_type = outcome<>;

	return a.apply_visitor(x3::make_lambda_visitor<result_type>(
		[&](const ast::rf::color_action& action) {
			return real_filter_add_color_action(st, action, set);
		},
		[&](const ast::rf::set_font_size_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::font_size_action{
					lang::integer{action.font_size.value},
					parser::position_tag_of(action)
				},
				set.set_font_size);
		},
		[&](const ast::rf::minimap_icon_action& action) -> result_type {
			return evaluate(action.icon).map_result([&](lang::minimap_icon icon) {
				return real_filter_add_action_impl(
					lang::minimap_icon_action{icon, parser::position_tag_of(action)},
					set.minimap_icon);
			});
		},
		[&](const ast::rf::play_effect_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::play_effect_action{
					lang::play_effect{evaluate(action.suit), action.is_temporary},
					parser::position_tag_of(action)
				},
				set.play_effect);
		},
		[&](const ast::rf::play_alert_sound_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::alert_sound_action{
					real_filter_make_builtin_alert_sound(action.id, action.volume, false),
					parser::position_tag_of(action)
				},
				set.play_alert_sound);
		},
		[&](const ast::rf::play_alert_sound_positional_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::alert_sound_action{
					real_filter_make_builtin_alert_sound(action.id, action.volume, true),
					parser::position_tag_of(action)
				},
				set.play_alert_sound);
		},
		[&](const ast::rf::custom_alert_sound_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::alert_sound_action{
					lang::alert_sound{lang::custom_alert_sound{evaluate(action.path)}},
					parser::position_tag_of(action)
				},
				set.play_alert_sound);
		},
		[&](const ast::rf::disable_drop_sound_action& action) -> result_type {
			return real_filter_add_action_impl(
				lang::disable_drop_sound_action{parser::position_tag_of(action)},
				set.disable_drop_sound);
		}
	));
}

} // namespace fs::compiler::detail
