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

void output_builtin_alert_sound(
	lang::builtin_alert_sound alert_sound,
	std::ostream& output_stream)
{
	output_stream << '\t';

	if (alert_sound.is_positional)
		output_stream << kw::play_alert_sound_positional;
	else
		output_stream << kw::play_alert_sound;

	output_stream << ' ' << alert_sound.sound_id.value;

	if (alert_sound.volume.has_value())
		output_stream << ' ' << (*alert_sound.volume).value;

	output_stream << '\n';
}

void output_custom_alert_sound(
	const lang::custom_alert_sound& alert_sound,
	std::ostream& output_stream)
{
	output_stream << '\t' << kw::custom_alert_sound
		<< " \"" << alert_sound.path.value << "\"\n";
}

void output_alert_sound(
	const std::optional<lang::alert_sound_action>& alert_sound_action,
	std::ostream& output_stream)
{
	if (!alert_sound_action.has_value())
		return;

	const lang::alert_sound& as = (*alert_sound_action).alert;
	std::visit(utility::visitor{
		[&output_stream](lang::builtin_alert_sound sound) {
			output_builtin_alert_sound(sound, output_stream);
		},
		[&output_stream](const lang::custom_alert_sound& sound) {
			output_custom_alert_sound(sound, output_stream);
		}
	}, as.sound);
}

void output_disabled_drop_sound(
	std::optional<lang::disable_drop_sound_action> action,
	std::ostream& output_stream)
{
	if (!action)
		return;

	output_stream << '\t' << kw::disable_drop_sound << '\n';
}

void output_suit(lang::suit s, std::ostream& output_stream)
{
	switch (s) {
		case lang::suit::red:
			output_stream << kw::red;
			break;
		case lang::suit::green:
			output_stream << kw::green;
			break;
		case lang::suit::blue:
			output_stream << kw::blue;
			break;
		case lang::suit::white:
			output_stream << kw::white;
			break;
		case lang::suit::brown:
			output_stream << kw::brown;
			break;
		case lang::suit::yellow:
			output_stream << kw::yellow;
			break;
		case lang::suit::cyan:
			output_stream << kw::cyan;
			break;
		case lang::suit::grey:
			output_stream << kw::grey;
			break;
		case lang::suit::orange:
			output_stream << kw::orange;
			break;
		case lang::suit::pink:
			output_stream << kw::pink;
			break;
		case lang::suit::purple:
			output_stream << kw::purple;
			break;
	}
}

void output_shape(lang::shape s, std::ostream& output_stream)
{
	switch (s) {
		case lang::shape::circle:
			output_stream << kw::circle;
			break;
		case lang::shape::diamond:
			output_stream << kw::diamond;
			break;
		case lang::shape::hexagon:
			output_stream << kw::hexagon;
			break;
		case lang::shape::square:
			output_stream << kw::square;
			break;
		case lang::shape::star:
			output_stream << kw::star;
			break;
		case lang::shape::triangle:
			output_stream << kw::triangle;
			break;
		case lang::shape::cross:
			output_stream << kw::cross;
			break;
		case lang::shape::moon:
			output_stream << kw::moon;
			break;
		case lang::shape::raindrop:
			output_stream << kw::raindrop;
			break;
		case lang::shape::kite:
			output_stream << kw::kite;
			break;
		case lang::shape::pentagon:
			output_stream << kw::pentagon;
			break;
		case lang::shape::upside_down_house:
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
	output_stream << '\t' << kw::minimap_icon << ' ' << mi.size.value << ' ';

	output_suit(mi.color, output_stream);
	output_stream << ' ';
	output_shape(mi.shape, output_stream);
	output_stream << '\n';
}

void output_beam_effect(
	std::optional<lang::play_effect_action> play_effect_action,
	std::ostream& output_stream)
{
	if (!play_effect_action.has_value())
		return;

	const lang::play_effect& effect = (*play_effect_action).beam;
	output_stream << '\t' << kw::play_effect << ' ';
	output_suit(effect.color, output_stream);
	if (effect.is_temporary)
		output_stream << ' ' << kw::temp;

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
	output_disabled_drop_sound(disable_drop_sound, output_stream);
	output_minimap_icon(minimap_icon, output_stream);
	output_beam_effect(play_effect, output_stream);
}

void action_set::override_with(action_set&& other)
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
		play_alert_sound = std::move(*other.play_alert_sound);

	disable_drop_sound = other.disable_drop_sound;

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
		&& lhs.disable_drop_sound == rhs.disable_drop_sound
		&& lhs.minimap_icon == rhs.minimap_icon
		&& lhs.play_effect == rhs.play_effect;
}

bool operator!=(const action_set& lhs, const action_set& rhs)
{
	return !(lhs == rhs);
}

}
