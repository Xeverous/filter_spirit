#include <fs/lang/action_set.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/visitor.hpp>

#include <ostream>

namespace fs::lang {

namespace {

void output_color_action(std::optional<color_action> action, const char* keyword, std::ostream& output_stream)
{
	if (!action.has_value())
		return;

	const color& c = (*action).c;
	output_stream << '\t' << keyword << ' ' << c.r.value << ' ' << c.g.value << ' ' << c.b.value;
	if (c.a.has_value())
		output_stream << ' ' << (*c.a).value;

	output_stream << '\n';
}

void output_font_size(std::optional<font_size_action> action, std::ostream& output_stream)
{
	if (!action.has_value())
		return;

	output_stream << '\t' << keywords::rf::set_font_size << ' ' << (*action).size.value << '\n';
}

void output_effect(std::optional<play_effect_action> action, std::ostream& output_stream)
{
	if (!action.has_value())
		return;

	output_stream << '\t' << keywords::rf::play_effect << ' ';

	std::visit(utility::visitor{
		[&](enabled_play_effect effect) {
			output_stream << to_string_view(effect.color.value);
			if (effect.is_temporary)
				output_stream << ' ' << keywords::rf::temp;
		},
		[&](disabled_play_effect /* effect */) {
			output_stream << keywords::rf::none;
		}
	}, (*action).effect);

	output_stream << '\n';
}

void output_minimap_icon(std::optional<minimap_icon_action> action, std::ostream& output_stream)
{
	if (!action.has_value())
		return;

	output_stream << '\t' << keywords::rf::minimap_icon << ' ';

	std::visit(utility::visitor{
		[&](enabled_minimap_icon icon) {
			output_stream << icon.size.value
				<< ' ' << to_string_view(icon.color.value)
				<< ' ' << to_string_view(icon.shape_.value);
		},
		[&](disabled_minimap_icon /* icon */) {
			output_stream << minimap_icon_action::sentinel_cancel_value;
		}
	}, (*action).icon);

	output_stream << '\n';
}

void output_builtin_alert_sound_id(builtin_alert_sound_id sound_id, std::ostream& output_stream)
{
	output_stream << ' ';

	std::visit(utility::visitor{
		[&output_stream](none_type /* sound_id */)   { output_stream << keywords::rf::none; },
		[&output_stream](integer sound_id)           { output_stream << sound_id.value; },
		[&output_stream](shaper_voice_line sound_id) { output_stream << to_string_view(sound_id.value); }
	}, sound_id.id);
}

void output_alert_sound(const std::optional<alert_sound_action>& action, std::ostream& output_stream)
{
	if (!action.has_value())
		return;

	output_stream << '\t';

	const alert_sound_action& act = *action;

	std::visit(utility::visitor{
		[&output_stream](builtin_alert_sound sound) {
			if (sound.is_positional)
				output_stream << keywords::rf::play_alert_sound_positional;
			else
				output_stream << keywords::rf::play_alert_sound;

			output_builtin_alert_sound_id(sound.sound_id, output_stream);
		},
		[&output_stream](const custom_alert_sound& sound) {
			if (sound.is_optional)
				output_stream << keywords::rf::custom_alert_sound_optional;
			else
				output_stream << keywords::rf::custom_alert_sound;

			output_stream << " \"" << sound.path.value << '\"';
		}
	}, act.sound);

	if (act.volume && !act.is_disabled())
		output_stream << ' ' << (*act.volume).value;

	output_stream << '\n';
}

void output_switch_drop_sound(
	std::optional<switch_drop_sound_action> action, bool if_alert_sound, std::ostream& output_stream)
{
	if (!action)
		return;

	output_stream << '\t';

	const switch_drop_sound_action& act = *action;

	if (if_alert_sound) {
		if (act.enable)
			output_stream << keywords::rf::enable_drop_sound_if_alert_sound;
		else
			output_stream << keywords::rf::disable_drop_sound_if_alert_sound;
	}
	else {
		if (act.enable)
			output_stream << keywords::rf::enable_drop_sound;
		else
			output_stream << keywords::rf::disable_drop_sound;
	}

	output_stream << '\n';
}

} // namespace

void action_set::print(std::ostream& output_stream) const
{
	output_color_action(text_color,       keywords::rf::set_text_color,       output_stream);
	output_color_action(border_color,     keywords::rf::set_border_color,     output_stream);
	output_color_action(background_color, keywords::rf::set_background_color, output_stream);

	output_font_size(font_size, output_stream);
	output_effect(effect, output_stream);
	output_minimap_icon(minimap_icon, output_stream);

	output_alert_sound(alert_sound, output_stream);
	output_switch_drop_sound(switch_drop_sound,                false, output_stream);
	output_switch_drop_sound(switch_drop_sound_if_alert_sound, true,  output_stream);
}

void action_set::override_with(const action_set& other)
{
	if (other.text_color)
		text_color = *other.text_color;

	if (other.border_color)
		border_color = *other.border_color;

	if (other.background_color)
		background_color = *other.background_color;

	if (other.font_size)
		font_size = *other.font_size;

	if (other.effect)
		effect = *other.effect;

	if (other.minimap_icon)
		minimap_icon = *other.minimap_icon;

	if (other.alert_sound)
		alert_sound = *other.alert_sound;

	if (other.switch_drop_sound)
		switch_drop_sound = *other.switch_drop_sound;

	if (other.switch_drop_sound_if_alert_sound)
		switch_drop_sound_if_alert_sound = *other.switch_drop_sound_if_alert_sound;
}

}
