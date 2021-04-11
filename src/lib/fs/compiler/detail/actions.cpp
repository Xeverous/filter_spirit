#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/lang/limits.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/monadic.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <boost/optional.hpp>

#include <utility>
#include <algorithm>
#include <iterator>
#include <type_traits>

namespace ast = fs::parser::ast;
namespace x3 = boost::spirit::x3;

namespace
{

using namespace fs;
using namespace fs::compiler;

using dmid = diagnostic_message_id;

// ---- generic helpers ----

[[nodiscard]] boost::optional<lang::color>
make_color(
	settings /* st */,
	lang::integer r,
	lang::integer g,
	lang::integer b,
	boost::optional<lang::integer> a,
	diagnostics_container& diagnostics)
{
	const auto result_r = detail::make_integer_in_range(r, 0, 255, diagnostics);
	const auto result_g = detail::make_integer_in_range(g, 0, 255, diagnostics);
	const auto result_b = detail::make_integer_in_range(b, 0, 255, diagnostics);

	if (!result_r || !result_g || !result_b)
		return boost::none;

	if (a) {
		const auto result_a = detail::make_integer_in_range(*a, 0, 255, diagnostics);

		if (!result_a)
			return boost::none;

		return lang::color{*result_r, *result_g, *result_b, *result_a};
	}
	else {
		return lang::color{*result_r, *result_g, *result_b};
	}
}

[[nodiscard]] boost::optional<lang::enabled_minimap_icon>
make_enabled_minimap_icon(
	lang::integer size,
	lang::suit suit,
	lang::shape shape,
	diagnostics_container& diagnostics)
{
	if (size.value != 0 && size.value != 1 && size.value != 2) {
		push_error_invalid_integer_value(0, 2, size, diagnostics);
		return boost::none;
	}

	return lang::enabled_minimap_icon{size, suit, shape};
}

[[nodiscard]] boost::optional<lang::minimap_icon>
make_minimap_icon(
	settings /* st */,
	const lang::object& obj,
	diagnostics_container& diagnostics)
{
	FS_ASSERT(obj.values.size() >= 1u);
	FS_ASSERT(obj.values.size() <= 3u);

	return detail::get_as<lang::integer>(obj.values[0], diagnostics)
		.flat_map([&](lang::integer icon_size) -> boost::optional<lang::minimap_icon>
		{
			if (icon_size.value == lang::minimap_icon::sentinel_cancel_value)
				return lang::minimap_icon{lang::disabled_minimap_icon{icon_size.origin}, obj.origin};

			// TODO this is somewhat misleading, the expectations are:
			// A) 1-3 arguments where first is integer{-1}
			// B)   3 arguments where [0] is integer, [1] is suit, [2] is shape
			if (obj.values.size() != 3u) {
				push_error_invalid_amount_of_arguments(
					3, 3, static_cast<int>(obj.values.size()), obj.origin, diagnostics);
				return boost::none;
			}

			FS_ASSERT(obj.values.size() == 3u);

			auto suit = detail::get_as<lang::suit>(obj.values[1], diagnostics);
			auto shape = detail::get_as<lang::shape>(obj.values[2], diagnostics);

			if (!suit || !shape)
				return boost::none;

			return make_enabled_minimap_icon(icon_size, *suit, *shape, diagnostics)
				.map([&](lang::enabled_minimap_icon icon) {
					return lang::minimap_icon{icon, obj.origin};
				});
		});
}

[[nodiscard]] boost::optional<lang::play_effect>
make_play_effect(
	settings /* st */,
	const lang::object& obj,
	diagnostics_container& diagnostics)
{
	FS_ASSERT(obj.values.size() >= 1u);
	FS_ASSERT(obj.values.size() <= 2u);

	diagnostics_container diagnostics_none;
	auto none = detail::get_as<lang::none>(obj.values[0], diagnostics_none);
	diagnostics_container diagnostics_suit;
	auto suit = detail::get_as<lang::suit>(obj.values[0], diagnostics_suit);

	if (none.has_value()) {
		std::move(diagnostics_none.begin(), diagnostics_none.end(), std::back_inserter(diagnostics));
		return lang::play_effect{lang::disabled_play_effect{*none}, obj.origin};
	}
	else if (suit.has_value()) {
		std::move(diagnostics_suit.begin(), diagnostics_suit.end(), std::back_inserter(diagnostics));

		const auto num_values = obj.values.size();
		FS_ASSERT(num_values == 1u || num_values == 2u);

		auto is_temp = [&]() -> boost::optional<bool> {
			if (num_values != 2)
				return false; // no "Temp" token, therefore false

			FS_ASSERT(num_values == 2u);

			// there is an extra token, it must be "Temp"; if successful map to true
			return detail::get_as<lang::temp>(obj.values[1], diagnostics)
				.map([](lang::temp) { return true; });
		}();

		if (!is_temp)
			return boost::none;

		return lang::play_effect{lang::enabled_play_effect{*suit, *is_temp}, obj.origin};
	}
	else {
		// main error
		diagnostics.push_back(make_error(dmid::invalid_play_effect, obj.origin, "invalid ", lang::keywords::rf::play_effect));
		// errors specifying each variant problems
		diagnostics.push_back(make_note_attempt_description("in attempt of None"));
		std::move(diagnostics_none.begin(), diagnostics_none.end(), std::back_inserter(diagnostics));
		diagnostics.push_back(make_note_attempt_description("in attempt of Suit [Temp]"));
		std::move(diagnostics_suit.begin(), diagnostics_suit.end(), std::back_inserter(diagnostics));
		return boost::none;
	}
}

[[nodiscard]] boost::optional<lang::builtin_alert_sound_id>
make_builtin_alert_sound_id(
	const lang::single_object& sobj,
	diagnostics_container& diagnostics)
{
	diagnostics_container diagnostics_integer;
	auto integer_sound_id = detail::get_as<lang::integer>(sobj, diagnostics_integer);
	diagnostics_container diagnostics_shaper;
	auto shaper_sound_id = detail::get_as<lang::shaper_voice_line>(sobj, diagnostics_shaper);

	if (integer_sound_id.has_value()) {
		std::move(diagnostics_integer.begin(), diagnostics_integer.end(), std::back_inserter(diagnostics));
		return detail::make_integer_in_range(
			*integer_sound_id,
			lang::limits::min_filter_sound_id,
			lang::limits::max_filter_sound_id,
			diagnostics)
			.map([](lang::integer sound_id) {
				return lang::builtin_alert_sound_id{sound_id};
			});
	}
	else if (shaper_sound_id.has_value()) {
		std::move(diagnostics_shaper.begin(), diagnostics_shaper.end(), std::back_inserter(diagnostics));
		return lang::builtin_alert_sound_id(*shaper_sound_id);
	}
	else {
		// main error
		diagnostics.push_back(make_error(dmid::invalid_alert_sound_id, sobj.origin, "invalid value for alert sound ID"));
		// errors specifying each sound ID variant problems
		diagnostics.push_back(make_note_attempt_description("in attempt of integer ID"));
		std::move(diagnostics_integer.begin(), diagnostics_integer.end(), std::back_inserter(diagnostics));
		diagnostics.push_back(make_note_attempt_description("in attempt of Shaper voice line ID"));
		std::move(diagnostics_shaper.begin(), diagnostics_shaper.end(), std::back_inserter(diagnostics));
		return boost::none;
	}
}

[[nodiscard]] boost::optional<lang::builtin_alert_sound>
make_builtin_alert_sound(
	settings /* st */,
	bool is_positional,
	const lang::single_object& sobj,
	diagnostics_container& diagnostics)
{
	return make_builtin_alert_sound_id(sobj, diagnostics)
		.map([&](lang::builtin_alert_sound_id sound_id) {
			return lang::builtin_alert_sound{is_positional, sound_id};
		});
}

[[nodiscard]] boost::optional<lang::custom_alert_sound>
make_custom_alert_sound(
	settings /* st */,
	const lang::single_object& sobj,
	diagnostics_container& diagnostics)
{
	return detail::get_as<lang::string>(sobj, diagnostics)
		.map([](lang::string str) {
			return lang::custom_alert_sound{std::move(str)};
		});
}

[[nodiscard]] boost::optional<lang::volume>
make_volume(
	settings /* st */,
	lang::integer intgr,
	diagnostics_container& diagnostics)
{
	return detail::make_integer_in_range(
		intgr,
		lang::limits::min_filter_volume,
		lang::limits::max_filter_volume,
		diagnostics)
		.map([](lang::integer vol) {
			return lang::volume{vol};
		});
}

[[nodiscard]] boost::optional<lang::volume>
make_volume(
	settings st,
	lang::single_object sobj,
	diagnostics_container& diagnostics)
{
	return detail::get_as<lang::integer>(sobj, diagnostics)
		.flat_map([&](lang::integer intgr) {
			return make_volume(st, intgr, diagnostics);
		});
}

// ---- spirit filter helpers ----

[[nodiscard]] bool
spirit_filter_add_set_color_action_impl(
	lang::color_action_type action_type,
	lang::color_action action,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	switch (action_type) {
		case lang::color_action_type::set_border_color: {
			set.set_border_color = action;
			return true;
		}
		case lang::color_action_type::set_text_color: {
			set.set_text_color = action;
			return true;
		}
		case lang::color_action_type::set_background_color: {
			set.set_background_color = action;
			return true;
		}
	}

	push_error_internal_compiler_error(__func__, action.origin, diagnostics);
	return false;
}

[[nodiscard]] bool
spirit_filter_add_set_color_action(
	settings st,
	const ast::sf::set_color_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 3, 4, diagnostics)
		.flat_map([&](lang::object obj) -> boost::optional<lang::color> {
			const auto num_values = obj.values.size();
			FS_ASSERT(num_values == 3u || num_values == 4u);

			const auto r = detail::get_as<lang::integer>(obj.values[0], diagnostics);
			const auto g = detail::get_as<lang::integer>(obj.values[1], diagnostics);
			const auto b = detail::get_as<lang::integer>(obj.values[2], diagnostics);

			if (!r || !g || !b)
				return boost::none;

			if (num_values == 4u) {
				return detail::get_as<lang::integer>(obj.values[3], diagnostics)
					.flat_map([&](lang::integer a) {
						return make_color(st, *r, *g, *b, a, diagnostics);
					}
				);
			}
			else {
				return make_color(st, *r, *g, *b, boost::none, diagnostics);
			}
		})
		.map([&](lang::color color) {
			auto act = lang::color_action{color, parser::position_tag_of(action)};
			return spirit_filter_add_set_color_action_impl(action.action_type, act, set, diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_set_font_size_action(
	settings st,
	const ast::sf::set_font_size_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 1, diagnostics)
		.flat_map([&](lang::object obj) {
			FS_ASSERT(obj.values.size() == 1u);
			return detail::get_as<lang::integer>(obj.values[0], diagnostics);
		})
		.map([&](lang::integer font_size) {
			if (font_size.value < lang::limits::min_filter_font_size
				|| font_size.value > lang::limits::max_filter_font_size)
			{
				diagnostics.push_back(make_diagnostic_message(
					make_warning_severity(st.error_handling),
					dmid::font_size_outside_range,
					font_size.origin,
					"font size outside allowed range (",
					std::to_string(lang::limits::min_filter_font_size),
					" - ",
					std::to_string(lang::limits::max_filter_font_size),
					")"));
			}

			return lang::font_size_action{font_size, parser::position_tag_of(action)};
		})
		.map([&](lang::font_size_action fsa) {
			set.set_font_size = fsa;
			return true;
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_minimap_icon_action(
	settings st,
	const ast::sf::minimap_icon_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 3, diagnostics)
		.flat_map([&](lang::object obj) {
			return make_minimap_icon(st, obj, diagnostics);
		})
		.map([&](lang::minimap_icon icon) {
			set.minimap_icon = lang::minimap_icon_action{icon, parser::position_tag_of(action)};
			return true;
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_play_effect_action(
	settings st,
	const ast::sf::play_effect_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 2, diagnostics)
		.flat_map([&](lang::object obj) {
			return make_play_effect(st, obj, diagnostics);
		})
		.map([&](lang::play_effect effect) {
			set.play_effect = lang::play_effect_action{effect, parser::position_tag_of(action)};
			return true;
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_play_alert_sound_action(
	settings st,
	const ast::sf::play_alert_sound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 2, diagnostics)
		.flat_map([&](lang::object obj) -> boost::optional<lang::alert_sound> {
			FS_ASSERT(obj.values.size() == 1u || obj.values.size() == 2u);
			auto bas = make_builtin_alert_sound(st, action.positional, obj.values[0], diagnostics);

			boost::optional<lang::volume> vol = lang::volume{};
			if (obj.values.size() == 2u) {
				vol = make_volume(st, obj.values[1], diagnostics);
			}

			if (bas && vol)
				return lang::alert_sound{*bas, *vol};
			else
				return boost::none;
		})
		.map([&](lang::alert_sound as) {
			set.play_alert_sound = lang::alert_sound_action{
				std::move(as),
				parser::position_tag_of(action)
			};
			return true;
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_custom_alert_sound_action(
	settings st,
	const ast::sf::custom_alert_sound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 2, diagnostics)
		.flat_map([&](lang::object obj) -> boost::optional<lang::alert_sound> {
			FS_ASSERT(obj.values.size() == 1u || obj.values.size() == 2u);
			auto cas = make_custom_alert_sound(st, obj.values[0], diagnostics);

			boost::optional<lang::volume> vol = lang::volume{};
			if (obj.values.size() == 2u) {
				vol = make_volume(st, obj.values[1], diagnostics);
			}

			if (cas && vol)
				return lang::alert_sound{std::move(*cas), *vol};
			else
				return boost::none;
		})
		.map([&](lang::alert_sound as) {
			set.play_alert_sound = lang::alert_sound_action{
				std::move(as),
				parser::position_tag_of(action)
			};
			return true;
		})
		.value_or(false);
}

[[nodiscard]] bool
spirit_filter_add_set_alert_sound_action(
	settings st,
	const ast::sf::set_alert_sound_action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_sequence(st, action.seq, symbols, 1, 2, diagnostics)
		.flat_map([&](lang::object obj) -> boost::optional<lang::alert_sound> {
			FS_ASSERT(obj.values.size() == 1u || obj.values.size() == 2u);

			// SetAlertSound always generates non-positional sounds, hence the false
			diagnostics_container diagnostics_builtin;
			auto builtin_alert = make_builtin_alert_sound(st, false, obj.values[0], diagnostics_builtin);
			diagnostics_container diagnostics_custom;
			auto custom_alert = make_custom_alert_sound(st, obj.values[0], diagnostics_custom);

			boost::optional<lang::volume> vol = lang::volume{};
			diagnostics_container diagnostics_volume;
			if (obj.values.size() == 2u) {
				vol = make_volume(st, obj.values[1], diagnostics_volume);
			}

			if (custom_alert.has_value() && vol.has_value()) {
				std::move(diagnostics_custom.begin(), diagnostics_custom.end(), std::back_inserter(diagnostics));
				std::move(diagnostics_volume.begin(), diagnostics_volume.end(), std::back_inserter(diagnostics));
				return lang::alert_sound{std::move(*custom_alert), *vol};
			}
			else if (builtin_alert.has_value() && vol.has_value()) {
				std::move(diagnostics_builtin.begin(), diagnostics_builtin.end(), std::back_inserter(diagnostics));
				std::move(diagnostics_volume.begin(), diagnostics_volume.end(), std::back_inserter(diagnostics));
				return lang::alert_sound{*builtin_alert, *vol};
			}
			else {
				diagnostics.push_back(make_error(
					dmid::invalid_set_alert_sound,
					parser::position_tag_of(action.seq),
					"invalid ",
					lang::keywords::sf::set_alert_sound));
				// errors specifying each sound variant problems
				diagnostics.push_back(make_note_attempt_description("in attempt of ", lang::keywords::rf::custom_alert_sound));
				std::move(diagnostics_custom.begin(), diagnostics_custom.end(), std::back_inserter(diagnostics));
				diagnostics.push_back(make_note_attempt_description("in attempt of ", lang::keywords::rf::play_alert_sound));
				std::move(diagnostics_builtin.begin(), diagnostics_builtin.end(), std::back_inserter(diagnostics));

				std::move(diagnostics_volume.begin(), diagnostics_volume.end(), std::back_inserter(diagnostics));
				return boost::none;
			}
		})
		.map([&](lang::alert_sound as) {
			set.play_alert_sound = lang::alert_sound_action{
				lang::alert_sound{std::move(as)},
				parser::position_tag_of(action)
			};
			return true;
		})
		.value_or(false);
}

// ---- real filter helpers ----

template <typename T>
[[nodiscard]] bool
real_filter_add_action_impl(
	T&& action,
	std::optional<T>& target,
	diagnostics_container& diagnostics)
{
	if (target) {
		diagnostics.push_back(make_error(
			dmid::action_redefinition,
			action.origin,
			"action redefinition (the same action can not be specified multiple times in the same block)"));
		diagnostics.push_back(make_note_first_defined_here((*target).origin));
		return false;
	}

	target = std::forward<T>(action);
	return true;
}

[[nodiscard]] boost::optional<lang::color>
real_filter_evaluate_color_literal(
	settings st,
	parser::ast::rf::color_literal cl,
	diagnostics_container& diagnostics)
{
	using detail::evaluate;
	boost::optional<lang::integer> a;
	if (cl.a)
		a = evaluate(*cl.a);

	return make_color(st, evaluate(cl.r), evaluate(cl.g), evaluate(cl.b), a, diagnostics);
}

[[nodiscard]] bool
real_filter_add_color_action(
	settings st,
	const ast::rf::color_action& action,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	boost::optional<lang::color> color = real_filter_evaluate_color_literal(st, action.color, diagnostics);

	if (!color)
		return false;

	auto act = lang::color_action{*color, parser::position_tag_of(action)};

	switch (action.action) {
		case lang::color_action_type::set_border_color: {
			return real_filter_add_action_impl(std::move(act), set.set_border_color, diagnostics);
		}
		case lang::color_action_type::set_text_color: {
			return real_filter_add_action_impl(std::move(act), set.set_text_color, diagnostics);
		}
		case lang::color_action_type::set_background_color: {
			return real_filter_add_action_impl(std::move(act), set.set_background_color, diagnostics);
		}
	}

	push_error_internal_compiler_error(__func__, act.origin, diagnostics);
	return false;
}

[[nodiscard]] bool
real_filter_add_minimap_icon_action(
	settings st,
	const ast::rf::minimap_icon_action& action,
	std::optional<lang::minimap_icon_action>& target,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_literal_sequence(st, action.literals, 1, 3, diagnostics)
		.flat_map([&](lang::object obj) {
			return make_minimap_icon(st, obj, diagnostics);
		})
		.map([&](lang::minimap_icon icon) {
			target = lang::minimap_icon_action{icon, parser::position_tag_of(action)};
			return true;
		})
		.value_or(false);
}

[[nodiscard]] bool
real_filter_add_play_effect_action(
	settings st,
	const ast::rf::play_effect_action& action,
	std::optional<lang::play_effect_action>& target,
	diagnostics_container& diagnostics)
{
	return detail::evaluate_literal_sequence(st, action.literals, 1, 2, diagnostics)
		.flat_map([&](lang::object obj) {
			return make_play_effect(st, obj, diagnostics);
		})
		.map([&](lang::play_effect effect) {
			target = lang::play_effect_action{effect, parser::position_tag_of(action)};
			return true;
		})
		.value_or(false);
}

[[nodiscard]] boost::optional<lang::volume>
real_filter_make_volume(
	settings st,
	boost::optional<ast::rf::integer_literal> maybe_integer,
	diagnostics_container& diagnostics)
{
	if (!maybe_integer)
		return lang::volume{};

	return maybe_integer.flat_map([&](ast::rf::integer_literal il) {
		return make_volume(st, detail::evaluate(il), diagnostics);
	});
}

[[nodiscard]] bool
real_filter_add_play_alert_sound_action(
	settings st,
	const ast::rf::play_alert_sound_action& action,
	std::optional<lang::alert_sound_action>& target,
	diagnostics_container& diagnostics)
{
	auto sobj = detail::evaluate(st, action.id, diagnostics);
	auto volume = real_filter_make_volume(st, action.volume, diagnostics);

	if (!sobj || !volume)
		return false;

	return make_builtin_alert_sound(st, action.positional, *sobj, diagnostics)
		.map([&](lang::builtin_alert_sound bas) {
			return real_filter_add_action_impl(
				lang::alert_sound_action{
					lang::alert_sound{bas, *volume},
					parser::position_tag_of(action)
				},
				target,
				diagnostics);
		})
		.value_or(false);
}

[[nodiscard]] bool
real_filter_add_custom_alert_sound_action(
	settings st,
	const ast::rf::custom_alert_sound_action& action,
	std::optional<lang::alert_sound_action>& target,
	diagnostics_container& diagnostics)
{
	return real_filter_make_volume(st, action.volume, diagnostics)
		.map([&](lang::volume vol) {
			return real_filter_add_action_impl(
				lang::alert_sound_action{
					lang::alert_sound{lang::custom_alert_sound{detail::evaluate(action.path)}, vol},
					parser::position_tag_of(action)
				},
				target,
				diagnostics);
		})
		.value_or(false);
}

} // namespace

namespace fs::compiler::detail
{

bool
spirit_filter_add_action(
	settings st,
	const ast::sf::action& action,
	const lang::symbol_table& symbols,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	return action.apply_visitor(x3::make_lambda_visitor<bool>(
		[&](const ast::sf::set_color_action& a) {
			return spirit_filter_add_set_color_action(st, a, symbols, set, diagnostics);
		},
		[&](const ast::sf::set_font_size_action& a) {
			return spirit_filter_add_set_font_size_action(st, a, symbols, set, diagnostics);
		},
		[&](const ast::sf::minimap_icon_action& a) {
			return spirit_filter_add_minimap_icon_action(st, a, symbols, set, diagnostics);
		},
		[&](const ast::sf::play_effect_action& a) {
			return spirit_filter_add_play_effect_action(st, a, symbols, set, diagnostics);
		},
		[&](const ast::sf::play_alert_sound_action& a) {
			return spirit_filter_add_play_alert_sound_action(st, a, symbols, set, diagnostics);
		},
		[&](const ast::sf::custom_alert_sound_action& a) {
			return spirit_filter_add_custom_alert_sound_action(st, a, symbols, set, diagnostics);
		},
		[&](const ast::sf::set_alert_sound_action& a) {
			return spirit_filter_add_set_alert_sound_action(st, a, symbols, set, diagnostics);
		},
		[&](const ast::sf::switch_drop_sound_action& a) {
			set.switch_drop_sound = lang::switch_drop_sound_action{a.enable, parser::position_tag_of(action)};
			return true;
		}
	));
}

bool
real_filter_add_action(
	settings st,
	const parser::ast::rf::action& a,
	lang::action_set& set,
	diagnostics_container& diagnostics)
{
	return a.apply_visitor(x3::make_lambda_visitor<bool>(
		[&](const ast::rf::color_action& action) {
			return real_filter_add_color_action(st, action, set, diagnostics);
		},
		[&](const ast::rf::set_font_size_action& action) {
			return real_filter_add_action_impl(
				lang::font_size_action{
					lang::integer{action.font_size.value},
					parser::position_tag_of(action)
				},
				set.set_font_size,
				diagnostics);
		},
		[&](const ast::rf::minimap_icon_action& action) {
			return real_filter_add_minimap_icon_action(st, action, set.minimap_icon, diagnostics);
		},
		[&](const ast::rf::play_effect_action& action) {
			return real_filter_add_play_effect_action(st, action, set.play_effect, diagnostics);
		},
		[&](const ast::rf::play_alert_sound_action& action) {
			return real_filter_add_play_alert_sound_action(st, action, set.play_alert_sound, diagnostics);
		},
		[&](const ast::rf::custom_alert_sound_action& action) {
			return real_filter_add_custom_alert_sound_action(st, action, set.play_alert_sound, diagnostics);
		},
		[&](const ast::rf::switch_drop_sound_action& action) {
			return real_filter_add_action_impl(
				lang::switch_drop_sound_action{action.enable, parser::position_tag_of(action)},
				set.switch_drop_sound,
				diagnostics);
		}
	));
}

} // namespace fs::compiler::detail
