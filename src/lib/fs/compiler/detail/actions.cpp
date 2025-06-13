#include <fs/compiler/detail/evaluate.hpp>
#include <fs/compiler/detail/actions.hpp>
#include <fs/lang/limits.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/lang/action_set.hpp>
#include <fs/utility/assert.hpp>

#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <boost/optional.hpp>

#include <utility>
#include <type_traits>

namespace fs::compiler::detail {
namespace {

namespace ast = parser::ast;
using parser::position_tag_of;
using dmid = diagnostic_message_id;
using boost::spirit::x3::make_lambda_visitor;

// ---- lang object creation helpers ----

[[nodiscard]] boost::optional<lang::color>
make_color(
	settings st,
	lang::integer r,
	lang::integer g,
	lang::integer b,
	boost::optional<lang::integer> a,
	bool is_set_text_color,
	diagnostics_store& diagnostics)
{
	const auto result_r = make_integer_in_range(r, 0, 255, diagnostics);
	const auto result_g = make_integer_in_range(g, 0, 255, diagnostics);
	const auto result_b = make_integer_in_range(b, 0, 255, diagnostics);

	if (!result_r || !result_g || !result_b)
		return boost::none;

	if (a) {
		const auto result_a = make_integer_in_range(*a, 0, 255, diagnostics);

		if (!result_a)
			return boost::none;

		lang::integer alpha = *result_a;
		if (is_set_text_color && st.ruthless_mode
			&& alpha.value < lang::limits::ruthless_min_set_text_color_alpha)
		{
			diagnostics.push_error_value_out_of_range(
				lang::limits::ruthless_min_set_text_color_alpha, 255, alpha);
			diagnostics.push_message(make_note_minor(
				alpha.origin,
				"Ruthless filters require SetTextColor alpha value to be at least ",
				std::to_string(lang::limits::ruthless_min_set_text_color_alpha)));

			if (st.error_handling.stop_on_error)
				return boost::none;
			else
				alpha.value = lang::limits::ruthless_min_set_text_color_alpha;
		}

		return lang::color{*result_r, *result_g, *result_b, alpha};
	}
	else {
		return lang::color{*result_r, *result_g, *result_b, std::nullopt};
	}
}

[[nodiscard]] boost::optional<lang::color>
make_color(settings st, const lang::object& obj, bool is_set_text_color, diagnostics_store& diagnostics)
{
	if (!check_object_size(obj, 3, 4, diagnostics))
		return boost::none;

	const auto r = get_as<lang::integer>(obj.values[0], diagnostics);
	const auto g = get_as<lang::integer>(obj.values[1], diagnostics);
	const auto b = get_as<lang::integer>(obj.values[2], diagnostics);

	if (!r || !g || !b)
		return boost::none;

	if (obj.values.size() == 4u) {
		return get_as<lang::integer>(obj.values[3], diagnostics)
			.flat_map([&](lang::integer a) {
				return make_color(st, *r, *g, *b, a, is_set_text_color, diagnostics);
			}
		);
	}
	else {
		return make_color(st, *r, *g, *b, boost::none, is_set_text_color, diagnostics);
	}

}

[[nodiscard]] boost::optional<lang::play_effect_action>
make_play_effect_action(
	settings /* st */,
	const lang::object& obj,
	lang::position_tag action_origin,
	diagnostics_store& diagnostics)
{
	if (!check_object_size(obj, 1, 2, diagnostics))
		return boost::none;

	diagnostics_store diagnostics_none;
	auto none = get_as<lang::none_type>(obj.values[0], diagnostics_none);
	diagnostics_store diagnostics_suit;
	auto suit = get_as<lang::suit>(obj.values[0], diagnostics_suit);

	if (none.has_value()) {
		diagnostics.move_messages_from(diagnostics_none);
		return lang::play_effect_action{lang::disabled_play_effect{*none}, action_origin};
	}
	else if (suit.has_value()) {
		diagnostics.move_messages_from(diagnostics_suit);

		const auto num_values = obj.values.size();
		FS_ASSERT(num_values == 1u || num_values == 2u);

		auto is_temp = [&]() -> boost::optional<bool> {
			if (num_values != 2)
				return false; // no "Temp" token, therefore false

			FS_ASSERT(num_values == 2u);

			// there is an extra token, it must be "Temp"; if successful map to true
			return get_as<lang::temp_type>(obj.values[1], diagnostics)
				.map([](lang::temp_type) { return true; });
		}();

		if (!is_temp)
			return boost::none;

		return lang::play_effect_action{lang::enabled_play_effect{*suit, *is_temp}, action_origin};
	}
	else {
		// main error
		diagnostics.push_message(make_error(
			dmid::invalid_play_effect, obj.origin, "invalid ", lang::keywords::rf::play_effect));
		// errors specifying each variant problems
		diagnostics.push_message(make_note_attempt_description("in attempt of None"));
		diagnostics.move_messages_from(diagnostics_none);
		diagnostics.push_message(make_note_attempt_description("in attempt of Suit [Temp]"));
		diagnostics.move_messages_from(diagnostics_suit);
		return boost::none;
	}
}

[[nodiscard]] boost::optional<lang::minimap_icon_action>
make_minimap_icon_action(
	settings /* st */,
	const lang::object& obj,
	lang::position_tag action_origin,
	diagnostics_store& diagnostics)
{
	if (!check_object_size(obj, 1, 3, diagnostics))
		return boost::none;

	auto maybe_icon_size = get_as<lang::integer>(obj.values[0], diagnostics);
	if (!maybe_icon_size) {
		// Check for a common mistake: using None instead of -1 for MinimapIcon
		diagnostics_store diagnostics_ignored;
		auto maybe_none = get_as<lang::none_type>(obj.values[0], diagnostics_ignored);
		if (maybe_none) {
			diagnostics.push_message(make_note(
				dmid::minor_note, obj.values[0].origin, "use value -1 to disable MinimapIcon"));
		}

		return boost::none;
	}

	const lang::integer& icon_size = *maybe_icon_size;

	// Intentionally ignore other arguments if size specifies disabled action.
	// This allows to configure enable/disable through a simple variable.
	if (icon_size.value == lang::minimap_icon_action::sentinel_cancel_value)
		return lang::minimap_icon_action{lang::disabled_minimap_icon{icon_size.origin}, action_origin};

	// Now return error if there are no 3 objects.
	if (obj.values.size() != 3u) {
		diagnostics.push_error_invalid_amount_of_arguments(
			3, 3, static_cast<int>(obj.values.size()), obj.origin);
		return boost::none;
	}

	FS_ASSERT(obj.values.size() == 3u);
	auto maybe_suit = get_as<lang::suit>(obj.values[1], diagnostics);
	auto maybe_shape = get_as<lang::shape>(obj.values[2], diagnostics);

	if (!maybe_suit || !maybe_shape)
		return boost::none;

	if (icon_size.value != 0 && icon_size.value != 1 && icon_size.value != 2) {
		diagnostics.push_error_value_out_of_range(0, 2, icon_size);
		return boost::none;
	}

	return lang::minimap_icon_action{
		lang::enabled_minimap_icon{icon_size, *maybe_suit, *maybe_shape},
		action_origin
	};
}

[[nodiscard]] boost::optional<lang::builtin_alert_sound_id>
make_builtin_alert_sound_id(
	const lang::single_object& sobj,
	diagnostics_store& diagnostics)
{
	diagnostics_store diagnostics_none;
	auto none_sound_id = get_as<lang::none_type>(sobj, diagnostics_none);
	diagnostics_store diagnostics_integer;
	auto integer_sound_id = get_as<lang::integer>(sobj, diagnostics_integer);
	diagnostics_store diagnostics_shaper;
	auto shaper_sound_id = get_as<lang::shaper_voice_line>(sobj, diagnostics_shaper);

	if (none_sound_id.has_value()) {
		diagnostics.move_messages_from(diagnostics_none);
		return lang::builtin_alert_sound_id{*none_sound_id};
	}
	else if (integer_sound_id.has_value()) {
		diagnostics.move_messages_from(diagnostics_integer);
		return make_integer_in_range(
			*integer_sound_id,
			lang::limits::min_filter_sound_id,
			lang::limits::max_filter_sound_id,
			diagnostics)
			.map([](lang::integer sound_id) {
				return lang::builtin_alert_sound_id{sound_id};
			});
	}
	else if (shaper_sound_id.has_value()) {
		diagnostics.move_messages_from(diagnostics_shaper);
		return lang::builtin_alert_sound_id{*shaper_sound_id};
	}
	else {
		// main error
		diagnostics.push_message(make_error(
			dmid::invalid_alert_sound_id, sobj.origin, "invalid value for alert sound ID"));
		// errors specifying each sound ID variant problems
		diagnostics.push_message(make_note_attempt_description("in attempt of None (disabled action)"));
		diagnostics.move_messages_from(diagnostics_none);
		diagnostics.push_message(make_note_attempt_description("in attempt of integer ID"));
		diagnostics.move_messages_from(diagnostics_integer);
		diagnostics.push_message(make_note_attempt_description("in attempt of Shaper voice line ID"));
		diagnostics.move_messages_from(diagnostics_shaper);
		return boost::none;
	}
}

[[nodiscard]] boost::optional<lang::builtin_alert_sound>
make_builtin_alert_sound(
	settings /* st */,
	bool is_positional,
	const lang::single_object& sobj,
	diagnostics_store& diagnostics)
{
	return make_builtin_alert_sound_id(sobj, diagnostics)
		.map([&](lang::builtin_alert_sound_id sound_id) {
			return lang::builtin_alert_sound{is_positional, sound_id};
		});
}

[[nodiscard]] boost::optional<lang::custom_alert_sound>
make_custom_alert_sound(
	settings /* st */,
	bool optional,
	const lang::single_object& sobj,
	diagnostics_store& diagnostics)
{
	return get_as<lang::string>(sobj, diagnostics)
		.map([&](lang::string str) {
			return lang::custom_alert_sound{optional, std::move(str)};
		});
}

[[nodiscard]] boost::optional<lang::integer>
make_volume(
	settings /* st */,
	const lang::single_object& sobj,
	diagnostics_store& diagnostics)
{
	return get_as<lang::integer>(sobj, diagnostics)
		.flat_map([&](lang::integer intgr) {
			return make_integer_in_range(
				intgr,
				lang::limits::min_filter_volume,
				lang::limits::max_filter_volume,
				diagnostics);
		});
}

[[nodiscard]] boost::optional<lang::alert_sound_action>
make_builtin_alert_sound_action(
	settings st,
	bool is_positional,
	const lang::object& obj,
	lang::position_tag action_origin,
	diagnostics_store& diagnostics)
{
	if (!check_object_size(obj, 1, 2, diagnostics))
		return boost::none;

	auto maybe_bas = make_builtin_alert_sound(st, is_positional, obj.values[0], diagnostics);
	if (!maybe_bas)
		return boost::none;

	std::optional<lang::integer> volume;
	if (obj.values.size() == 2u) {
		auto maybe_volume = make_volume(st, obj.values[1], diagnostics);

		if (maybe_volume)
			volume = *maybe_volume;
		else if (st.error_handling.stop_on_error)
			return boost::none;
	}

	return lang::alert_sound_action{*maybe_bas, volume, action_origin};
}

[[nodiscard]] boost::optional<lang::alert_sound_action>
make_custom_alert_sound_action(
	settings st,
	bool is_optional,
	const lang::object& obj,
	lang::position_tag action_origin,
	diagnostics_store& diagnostics)
{
	if (!check_object_size(obj, 1, 2, diagnostics))
		return boost::none;

	auto maybe_cas = make_custom_alert_sound(st, is_optional, obj.values[0], diagnostics);
	if (!maybe_cas)
		return boost::none;

	std::optional<lang::integer> volume;
	if (obj.values.size() == 2u) {
		auto maybe_volume = make_volume(st, obj.values[1], diagnostics);

		if (maybe_volume)
			volume = *maybe_volume;
		else if (st.error_handling.stop_on_error)
			return boost::none;
	}

	return lang::alert_sound_action{*maybe_cas, volume, action_origin};
}

[[nodiscard]] boost::optional<lang::alert_sound_action>
spirit_filter_make_set_alert_sound_action(
	settings st,
	const lang::object& obj,
	lang::position_tag action_origin,
	diagnostics_store& diagnostics)
{
	// SetAlertSound always generates non-positional built-in sounds, hence the false
	diagnostics_store diagnostics_builtin;
	auto maybe_builtin_alert = make_builtin_alert_sound_action(
		st, false, obj, action_origin, diagnostics_builtin);

	if (maybe_builtin_alert) {
		diagnostics.move_messages_from(diagnostics_builtin);
		return *maybe_builtin_alert;
	}

	// SetAlertSound always generates non-optional custom sounds, hence the false
	diagnostics_store diagnostics_custom;
	auto maybe_custom_alert = make_custom_alert_sound_action(
		st, false, obj, action_origin, diagnostics_custom);

	if (maybe_custom_alert) {
		diagnostics.move_messages_from(diagnostics_custom);
		return std::move(*maybe_custom_alert);
	}

	// if code reaches here, both have failed
	diagnostics.push_message(make_error(
		dmid::invalid_set_alert_sound,
		obj.origin,
		"invalid ",
		lang::keywords::sf::set_alert_sound));
	// errors specifying each sound variant problems
	diagnostics.push_message(make_note_attempt_description("in attempt of ", lang::keywords::rf::play_alert_sound));
	diagnostics.move_messages_from(diagnostics_builtin);
	diagnostics.push_message(make_note_attempt_description("in attempt of ", lang::keywords::rf::custom_alert_sound));
	diagnostics.move_messages_from(diagnostics_custom);
	return boost::none;
}

// ---- action addition helpers ----

[[nodiscard]] bool
add_set_color_action(
	settings st,
	const lang::object& obj,
	lang::position_tag action_origin,
	std::optional<lang::color_action>& target,
	bool is_set_text_color,
	diagnostics_store& diagnostics)
{
	auto maybe_color = make_color(st, obj, is_set_text_color, diagnostics);
	if (!maybe_color)
		return false;

	target = lang::color_action{*maybe_color, action_origin};
	return true;
}

[[nodiscard]] bool
add_set_font_size_action(
	settings /* st */,
	const lang::object& obj,
	lang::position_tag action_origin,
	std::optional<lang::font_size_action>& target,
	diagnostics_store& diagnostics)
{
	if (!check_object_size(obj, 1, 1, diagnostics))
		return false;

	return get_as<lang::integer>(obj.values[0], diagnostics)
		.flat_map([&](lang::integer font_size) -> boost::optional<lang::font_size_action> {
			if (font_size.value < lang::limits::min_filter_font_size
				|| font_size.value > lang::limits::max_filter_font_size)
			{
				diagnostics.push_message(make_warning(
					diagnostic_message_id::value_out_of_range,
					font_size.origin,
					"invalid font size, expected value in range ",
					std::to_string(lang::limits::min_filter_font_size),
					" - ",
					std::to_string(lang::limits::max_filter_font_size),
					" but got ",
					std::to_string(font_size.value),
					" (game will clamp the font size in this range)"));
				return boost::none;
			}

			return lang::font_size_action{font_size, action_origin};
		})
		.map([&](lang::font_size_action fsa) {
			target = fsa;
			return true;
		})
		.value_or(false);
}

[[nodiscard]] bool
add_play_effect_action(
	settings st,
	const lang::object& obj,
	lang::position_tag action_origin,
	std::optional<lang::play_effect_action>& target,
	diagnostics_store& diagnostics)
{
	return make_play_effect_action(st, obj, action_origin, diagnostics)
		.map([&](lang::play_effect_action action) {
			target = action;
			return true;
		})
		.value_or(false);
}

[[nodiscard]] bool
add_minimap_icon_action(
	settings st,
	const lang::object& obj,
	lang::position_tag action_origin,
	std::optional<lang::minimap_icon_action>& target,
	diagnostics_store& diagnostics)
{
	return make_minimap_icon_action(st, obj, action_origin, diagnostics)
		.map([&](lang::minimap_icon_action action) {
			target = action;
			return true;
		})
		.value_or(false);
}

[[nodiscard]] bool
add_play_alert_sound_action(
	settings st,
	bool is_positional,
	const lang::object& obj,
	lang::position_tag action_origin,
	std::optional<lang::alert_sound_action>& target,
	diagnostics_store& diagnostics)
{
	auto maybe_basa = make_builtin_alert_sound_action(st, is_positional, obj, action_origin, diagnostics);
	if (!maybe_basa)
		return false;

	target = *maybe_basa;
	return true;
}

[[nodiscard]] bool
add_custom_alert_sound_action(
	settings st,
	bool is_optional,
	const lang::object& obj,
	lang::position_tag action_origin,
	std::optional<lang::alert_sound_action>& target,
	diagnostics_store& diagnostics)
{
	auto maybe_casa = make_custom_alert_sound_action(st, is_optional, obj, action_origin, diagnostics);
	if (!maybe_casa)
		return false;

	target = std::move(*maybe_casa);
	return true;
}

[[nodiscard]] bool
add_switch_drop_sound_action(
	settings /* st */,
	bool enable,
	const lang::object& obj,
	lang::position_tag action_origin,
	std::optional<lang::switch_drop_sound_action>& target,
	diagnostics_store& diagnostics)
{
	if (!check_object_size(obj, 0, 0, diagnostics))
		return false;

	target = lang::switch_drop_sound_action{enable, action_origin};
	return true;
}

void warn_if_action_exists(
	bool is_real_filter,
	lang::position_tag action_origin,
	std::optional<lang::position_tag> target_origin,
	diagnostics_store& diagnostics)
{
	// Warn only on real filters - spirit filter templates allow override by design.
	// In case of real filters (no action inheritance) any duplicate is a mistake.
	if (is_real_filter && target_origin) {
		diagnostics.push_message(make_warning(
			dmid::action_redefinition,
			action_origin,
			"duplicate action (will replace the previous one)"));
		diagnostics.push_message(make_note_first_defined_here(*target_origin));
	}
}

[[nodiscard]] bool
add_official_action(
	settings st,
	bool is_real_filter,
	lang::official_action_property property,
	const lang::object& obj,
	lang::position_tag action_origin,
	lang::action_set& set,
	diagnostics_store& diagnostics)
{
	auto origin_of = [](const auto& action) -> std::optional<lang::position_tag> {
		if (action.has_value())
			return (*action).origin;
		else
			return std::nullopt;
	};

	switch (property) {
		// text actions
		case lang::official_action_property::set_text_color:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.text_color), diagnostics);
			return add_set_color_action(st, obj, action_origin, set.text_color, true, diagnostics);
		case lang::official_action_property::set_border_color:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.border_color), diagnostics);
			return add_set_color_action(st, obj, action_origin, set.border_color, false, diagnostics);
		case lang::official_action_property::set_background_color:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.background_color), diagnostics);
			return add_set_color_action(st, obj, action_origin, set.background_color, false, diagnostics);
		// other simple actions
		case lang::official_action_property::set_font_size:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.font_size), diagnostics);
			return add_set_font_size_action(st, obj, action_origin, set.font_size, diagnostics);
		case lang::official_action_property::play_effect:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.effect), diagnostics);
			return add_play_effect_action(st, obj, action_origin, set.effect, diagnostics);
		case lang::official_action_property::minimap_icon:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.minimap_icon), diagnostics);
			return add_minimap_icon_action(st, obj, action_origin, set.minimap_icon, diagnostics);
		// alert sound actions
		case lang::official_action_property::play_alert_sound:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.alert_sound), diagnostics);
			return add_play_alert_sound_action(st, false, obj, action_origin, set.alert_sound, diagnostics);
		case lang::official_action_property::play_alert_sound_positional:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.alert_sound), diagnostics);
			return add_play_alert_sound_action(st, true, obj, action_origin, set.alert_sound, diagnostics);
		case lang::official_action_property::custom_alert_sound:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.alert_sound), diagnostics);
			return add_custom_alert_sound_action(st, false, obj, action_origin, set.alert_sound, diagnostics);
		case lang::official_action_property::custom_alert_sound_optional:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.alert_sound), diagnostics);
			return add_custom_alert_sound_action(st, true, obj, action_origin, set.alert_sound, diagnostics);
		// enable/disable alert sound actions
		case lang::official_action_property::disable_drop_sound:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.switch_drop_sound), diagnostics);
			return add_switch_drop_sound_action(st, false, obj, action_origin, set.switch_drop_sound, diagnostics);
		case lang::official_action_property::enable_drop_sound:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.switch_drop_sound), diagnostics);
			return add_switch_drop_sound_action(st, true, obj, action_origin, set.switch_drop_sound, diagnostics);
		case lang::official_action_property::disable_drop_sound_if_alert_sound:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.switch_drop_sound_if_alert_sound), diagnostics);
			return add_switch_drop_sound_action(st, false, obj, action_origin, set.switch_drop_sound_if_alert_sound, diagnostics);
		case lang::official_action_property::enable_drop_sound_if_alert_sound:
			warn_if_action_exists(is_real_filter, action_origin, origin_of(set.switch_drop_sound_if_alert_sound), diagnostics);
			return add_switch_drop_sound_action(st, true, obj, action_origin, set.switch_drop_sound_if_alert_sound, diagnostics);
	}

	diagnostics.push_error_internal_compiler_error(__func__, action_origin);
	return false;
}

[[nodiscard]] bool
spirit_filter_add_official_action(
	settings st,
	const ast::sf::official_action& action,
	const symbol_table& symbols,
	lang::action_set& set,
	diagnostics_store& diagnostics)
{
	auto maybe_obj = evaluate_sequence(st, action.seq, symbols, diagnostics);
	if (!maybe_obj)
		return false;

	return add_official_action(
		st, false, action.property, *maybe_obj, position_tag_of(action), set, diagnostics);
}

[[nodiscard]] bool
spirit_filter_add_set_alert_sound_action(
	settings st,
	const ast::sf::set_alert_sound_action& action,
	const symbol_table& symbols,
	lang::action_set& set,
	diagnostics_store& diagnostics)
{
	auto maybe_obj = evaluate_sequence(st, action.seq, symbols, diagnostics);
	if (!maybe_obj)
		return false;

	auto maybe_action = spirit_filter_make_set_alert_sound_action(
		st, *maybe_obj, position_tag_of(action), diagnostics);
	if (!maybe_action)
		return false;

	set.alert_sound = std::move(*maybe_action);
	return true;
}

} // namespace

bool real_filter_add_action(
	settings st,
	const ast::rf::action& action,
	lang::action_set& set,
	diagnostics_store& diagnostics)
{
	auto maybe_obj = evaluate_literal_sequence(st, action.seq, diagnostics);
	if (!maybe_obj)
		return false;

	return add_official_action(
		st, true, action.property, *maybe_obj, position_tag_of(action), set, diagnostics);
}

bool spirit_filter_add_action(
	settings st,
	const ast::sf::action& action,
	const symbol_table& symbols,
	lang::action_set& set,
	diagnostics_store& diagnostics)
{
	return action.apply_visitor(make_lambda_visitor<bool>(
		[&](const ast::sf::official_action& a) {
			return spirit_filter_add_official_action(st, a, symbols, set, diagnostics);
		},
		[&](const ast::sf::set_alert_sound_action& a) {
			return spirit_filter_add_set_alert_sound_action(st, a, symbols, set, diagnostics);
		}
	));
}

} // namespace fs::compiler::detail
