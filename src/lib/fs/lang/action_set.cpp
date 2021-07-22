#include <fs/lang/action_set.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/visitor.hpp>

#include <ostream>

namespace
{

using namespace fs;
namespace kw = lang::keywords::rf;

void output_color_action(
	std::optional<lang::color_action> color_action,
	const char* name,
	std::ostream& output_stream)
{
	if (!color_action.has_value())
		return;

	const lang::color& c = (*color_action).c;
	output_stream << '\t' << name << ' ' << c.r.value << ' ' << c.g.value << ' ' << c.b.value;
	if (c.a.has_value())
		output_stream << ' ' << (*c.a).value;

	output_stream << '\n';
}

void output_font_size(
	std::optional<lang::font_size_action> font_size_action,
	std::ostream& output_stream)
{
	if (!font_size_action.has_value())
		return;

	output_stream << '\t' << kw::set_font_size << ' ' << (*font_size_action).size.value << '\n';
}

void output_builtin_alert_sound_id(
	lang::builtin_alert_sound_id sound_id,
	std::ostream& output_stream)
{
	output_stream << ' ';

	std::visit(utility::visitor{
		[&output_stream](lang::none /* sound_id */) {
			output_stream << kw::none;
		},
		[&output_stream](lang::integer sound_id) {
			output_stream << sound_id.value;
		},
		[&output_stream](lang::shaper_voice_line sound_id) {
			switch (sound_id.value) {
				case lang::shaper_voice_line_type::mirror:
					output_stream << kw::sh_mirror;
					break;
				case lang::shaper_voice_line_type::exalted:
					output_stream << kw::sh_exalted;
					break;
				case lang::shaper_voice_line_type::divine:
					output_stream << kw::sh_divine;
					break;
				case lang::shaper_voice_line_type::general:
					output_stream << kw::sh_general;
					break;
				case lang::shaper_voice_line_type::regal:
					output_stream << kw::sh_regal;
					break;
				case lang::shaper_voice_line_type::chaos:
					output_stream << kw::sh_chaos;
					break;
				case lang::shaper_voice_line_type::fusing:
					output_stream << kw::sh_fusing;
					break;
				case lang::shaper_voice_line_type::alchemy:
					output_stream << kw::sh_alchemy;
					break;
				case lang::shaper_voice_line_type::vaal:
					output_stream << kw::sh_vaal;
					break;
				case lang::shaper_voice_line_type::blessed:
					output_stream << kw::sh_blessed;
					break;
			}
		}
	}, sound_id.id);
}

void output_alert_sound(
	const std::optional<lang::alert_sound_action>& alert_sound_action,
	std::ostream& output_stream)
{
	if (!alert_sound_action.has_value())
		return;

	output_stream << '\t';

	const lang::alert_sound& as = (*alert_sound_action).alert;
	std::visit(utility::visitor{
		[&output_stream](lang::builtin_alert_sound sound) {
			if (sound.is_positional)
				output_stream << kw::play_alert_sound_positional;
			else
				output_stream << kw::play_alert_sound;

			output_builtin_alert_sound_id(sound.sound_id, output_stream);
		},
		[&output_stream](const lang::custom_alert_sound& sound) {
			if (sound.optional)
				output_stream << kw::custom_alert_sound_optional;
			else
				output_stream << kw::custom_alert_sound;

			output_stream << " \"" << sound.path.value << '\"';
		}
	}, as.sound);

	if (as.vol.value && !as.is_disabled())
		output_stream << ' ' << (*as.vol.value).value;

	output_stream << '\n';
}

void output_switch_drop_sound(
	std::optional<lang::switch_drop_sound_action> action,
	std::ostream& output_stream)
{
	if (!action)
		return;

	output_stream << '\t';

	if (const lang::switch_drop_sound& sds = (*action).action; sds.enable) {
		if (sds.if_alert_sound)
			output_stream << kw::enable_drop_sound_if_alert;
		else
			output_stream << kw::enable_drop_sound;
	}
	else {
		if (sds.if_alert_sound)
			output_stream << kw::disable_drop_sound_if_alert;
		else
			output_stream << kw::disable_drop_sound;
	}

	output_stream << '\n';
}

void output_suit(lang::suit_type s, std::ostream& output_stream)
{
	switch (s) {
		case lang::suit_type::red:
			output_stream << kw::red;
			break;
		case lang::suit_type::green:
			output_stream << kw::green;
			break;
		case lang::suit_type::blue:
			output_stream << kw::blue;
			break;
		case lang::suit_type::white:
			output_stream << kw::white;
			break;
		case lang::suit_type::brown:
			output_stream << kw::brown;
			break;
		case lang::suit_type::yellow:
			output_stream << kw::yellow;
			break;
		case lang::suit_type::cyan:
			output_stream << kw::cyan;
			break;
		case lang::suit_type::grey:
			output_stream << kw::grey;
			break;
		case lang::suit_type::orange:
			output_stream << kw::orange;
			break;
		case lang::suit_type::pink:
			output_stream << kw::pink;
			break;
		case lang::suit_type::purple:
			output_stream << kw::purple;
			break;
	}
}

void output_shape(lang::shape_type s, std::ostream& output_stream)
{
	switch (s) {
		case lang::shape_type::circle:
			output_stream << kw::circle;
			break;
		case lang::shape_type::diamond:
			output_stream << kw::diamond;
			break;
		case lang::shape_type::hexagon:
			output_stream << kw::hexagon;
			break;
		case lang::shape_type::square:
			output_stream << kw::square;
			break;
		case lang::shape_type::star:
			output_stream << kw::star;
			break;
		case lang::shape_type::triangle:
			output_stream << kw::triangle;
			break;
		case lang::shape_type::cross:
			output_stream << kw::cross;
			break;
		case lang::shape_type::moon:
			output_stream << kw::moon;
			break;
		case lang::shape_type::raindrop:
			output_stream << kw::raindrop;
			break;
		case lang::shape_type::kite:
			output_stream << kw::kite;
			break;
		case lang::shape_type::pentagon:
			output_stream << kw::pentagon;
			break;
		case lang::shape_type::upside_down_house:
			output_stream << kw::upside_down_house;
			break;
	}
}

void output_minimap_icon(
	std::optional<lang::minimap_icon_action> minimap_icon_action,
	std::ostream& output_stream)
{
	if (!minimap_icon_action.has_value())
		return;

	const lang::minimap_icon& mi = (*minimap_icon_action).icon;
	output_stream << '\t' << kw::minimap_icon << ' ';

	std::visit(utility::visitor{
		[&](lang::enabled_minimap_icon icon) {
			output_stream << icon.size.value << ' ';
			output_suit(icon.color.value, output_stream);
			output_stream << ' ';
			output_shape(icon.shape_.value, output_stream);
		},
		[&](lang::disabled_minimap_icon /* icon */) {
			output_stream << lang::minimap_icon::sentinel_cancel_value;
		}
	}, mi.icon);

	output_stream << '\n';
}

void output_beam_effect(
	std::optional<lang::play_effect_action> play_effect_action,
	std::ostream& output_stream)
{
	if (!play_effect_action.has_value())
		return;

	const lang::play_effect& effect = (*play_effect_action).effect;
	output_stream << '\t' << kw::play_effect << ' ';

	std::visit(utility::visitor{
		[&](lang::enabled_play_effect effect) {
			output_suit(effect.color.value, output_stream);
			if (effect.is_temporary)
				output_stream << ' ' << kw::temp;
		},
		[&](lang::disabled_play_effect /* effect */) {
			output_stream << kw::none;
		}
	}, effect.effect);

	output_stream << '\n';
}

}

namespace fs::lang
{

void action_set::generate(std::ostream& output_stream) const
{
	output_color_action(set_border_color,     kw::set_border_color,     output_stream);
	output_color_action(set_text_color,       kw::set_text_color,       output_stream);
	output_color_action(set_background_color, kw::set_background_color, output_stream);

	output_font_size(set_font_size, output_stream);
	output_alert_sound(play_alert_sound, output_stream);
	output_switch_drop_sound(switch_drop_sound, output_stream);
	output_minimap_icon(minimap_icon, output_stream);
	output_beam_effect(play_effect, output_stream);
}

void action_set::override_with(const action_set& other)
{
	if (other.set_border_color)
		set_border_color = *other.set_border_color;

	if (other.set_text_color)
		set_text_color = *other.set_text_color;

	if (other.set_background_color)
		set_background_color = *other.set_background_color;

	if (other.set_font_size)
		set_font_size = *other.set_font_size;

	if (other.play_alert_sound)
		play_alert_sound = *other.play_alert_sound;

	if (other.switch_drop_sound)
		switch_drop_sound = *other.switch_drop_sound;

	if (other.minimap_icon)
		minimap_icon = *other.minimap_icon;

	if (other.play_effect)
		play_effect = *other.play_effect;
}

bool operator==(const action_set& lhs, const action_set& rhs)
{
	return lhs.set_border_color == rhs.set_border_color
		&& lhs.set_text_color == rhs.set_text_color
		&& lhs.set_background_color == rhs.set_background_color
		&& lhs.set_font_size == rhs.set_font_size
		&& lhs.play_alert_sound == rhs.play_alert_sound
		&& lhs.switch_drop_sound == rhs.switch_drop_sound
		&& lhs.minimap_icon == rhs.minimap_icon
		&& lhs.play_effect == rhs.play_effect;
}

bool operator!=(const action_set& lhs, const action_set& rhs)
{
	return !(lhs == rhs);
}

}
