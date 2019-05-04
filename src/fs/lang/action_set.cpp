#include "fs/lang/action_set.hpp"
#include "fs/lang/generation.hpp"
#include "fs/utility/visitor.hpp"

#include <ostream>

namespace
{

using namespace fs;

void output_color_action(
	std::optional<lang::color> color,
	const char* name,
	std::ostream& output_stream)
{
	if (!color.has_value())
		return;

	const lang::color& c = *color;
	output_stream << '\t' << name << ' ' << c.r << ' ' << c.g << ' ' << c.b;
	if (c.a.has_value())
		output_stream << ' ' << *c.a;

	output_stream << '\n';
}

void output_font_size(
	std::optional<lang::font_size> font_size,
	std::ostream& output_stream)
{
	if (!font_size.has_value())
		return;

	output_stream << '\t' << lang::generation::set_font_size << ' ' << (*font_size).value << '\n';
}

void output_alert_sound(
	const std::optional<lang::alert_sound>& alert_sound,
	std::ostream& output_stream)
{
	if (!alert_sound.has_value())
		return;

	const lang::alert_sound& as = *alert_sound;
	std::visit(utility::visitor{
		[&](lang::sound_id sid)
		{
			output_stream << '\t' << lang::generation::play_alert_sound << ' ' << sid.value;
		},
		[&](const lang::path& path)
		{
			output_stream << '\t' << lang::generation::custom_alert_sound << " \"" << path.value << '"';
		}
	}, as.sound);

	if (as.volume.has_value())
		output_stream << ' ' << (*as.volume).value;

	output_stream << '\n';
}

void output_disabled_drop_sound(
	bool is_disabled,
	std::ostream& output_stream)
{
	if (!is_disabled)
		return;

	output_stream << '\t' << lang::generation::disable_drop_sound << '\n';
}

void output_suit(lang::suit s, std::ostream& output_stream)
{
	switch (s)
	{
		case lang::suit::red:
			output_stream << lang::generation::red;
			break;
		case lang::suit::green:
			output_stream << lang::generation::green;
			break;
		case lang::suit::blue:
			output_stream << lang::generation::blue;
			break;
		case lang::suit::white:
			output_stream << lang::generation::white;
			break;
		case lang::suit::brown:
			output_stream << lang::generation::brown;
			break;
		case lang::suit::yellow:
			output_stream << lang::generation::yellow;
			break;
		default:
			break;
	}
}

void output_shape(lang::shape s, std::ostream& output_stream)
{
	switch (s)
	{
		case lang::shape::circle:
			output_stream << lang::generation::circle;
			break;
		case lang::shape::diamond:
			output_stream << lang::generation::diamond;
			break;
		case lang::shape::hexagon:
			output_stream << lang::generation::hexagon;
			break;
		case lang::shape::square:
			output_stream << lang::generation::square;
			break;
		case lang::shape::star:
			output_stream << lang::generation::star;
			break;
		case lang::shape::triangle:
			output_stream << lang::generation::triangle;
			break;
		default:
			break;
	}
}

void output_minimap_icon(
	std::optional<lang::minimap_icon> minimap_icon,
	std::ostream& output_stream)
{
	if (!minimap_icon.has_value())
		return;

	const lang::minimap_icon& mi = *minimap_icon;
	output_stream << '\t' << lang::generation::minimap_icon << ' ' << mi.size << ' ';

	output_suit(mi.color, output_stream);
	output_stream << ' ';
	output_shape(mi.shape, output_stream);
	output_stream << '\n';
}

void output_beam_effect(
	std::optional<lang::beam_effect> beam_effect,
	std::ostream& output_stream)
{
	if (!beam_effect.has_value())
		return;

	const lang::beam_effect& be = *beam_effect;
	output_stream << '\t' << lang::generation::play_effect << ' ';
	output_suit(be.color, output_stream);
	if (be.is_temporary)
		output_stream << lang::generation::temp;

	output_stream << '\n';
}

}

namespace fs::lang
{

void action_set::generate(std::ostream& output_stream) const
{
	namespace lg = lang::generation;
	output_color_action(border_color,     lg::set_border_color,     output_stream);
	output_color_action(text_color,       lg::set_text_color,       output_stream);
	output_color_action(background_color, lg::set_background_color, output_stream);

	output_font_size(font_size, output_stream);
	output_alert_sound(alert_sound, output_stream);
	output_disabled_drop_sound(disabled_drop_sound, output_stream);
	output_minimap_icon(minimap_icon, output_stream);
	output_beam_effect(beam_effect, output_stream);
}

action_set action_set::override_by(const action_set& other) const
{
	action_set result = *this;

	if (other.border_color)
		result.border_color = other.border_color;
	if (other.text_color)
		result.text_color = other.text_color;
	if (other.background_color)
		result.background_color = other.background_color;
	if (other.font_size)
		result.font_size = other.font_size;
	if (other.alert_sound)
		result.alert_sound = other.alert_sound;
	if (other.disabled_drop_sound)
		result.disabled_drop_sound = other.disabled_drop_sound;
	if (other.minimap_icon)
		result.minimap_icon = other.minimap_icon;
	if (other.beam_effect)
		result.beam_effect = other.beam_effect;

	return result;
}

}
