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

void output_built_in_alert_sound(
	lang::built_in_alert_sound alert_sound,
	std::ostream& output_stream)
{
	output_stream << '\t';

	if (alert_sound.is_positional.value)
		output_stream << lang::generation::play_alert_sound_positional;
	else
		output_stream << lang::generation::play_alert_sound;

	output_stream << ' ' << alert_sound.id.value;

	if (alert_sound.volume.has_value())
		output_stream << ' ' << (*alert_sound.volume).value;

	output_stream << '\n';
}

void output_custom_alert_sound(
	const lang::custom_alert_sound& alert_sound,
	std::ostream& output_stream)
{
	output_stream << '\t' << lang::generation::custom_alert_sound
		<< " \"" << alert_sound.path.value << "\"\n";
}

void output_alert_sound(
	const std::optional<lang::alert_sound>& alert_sound,
	std::ostream& output_stream)
{
	if (!alert_sound.has_value())
		return;

	const lang::alert_sound& as = *alert_sound;
	std::visit(utility::visitor{
		[&output_stream](lang::built_in_alert_sound sound)
		{
			output_built_in_alert_sound(sound, output_stream);
		},
		[&output_stream](const lang::custom_alert_sound& sound)
		{
			output_custom_alert_sound(sound, output_stream);
		}
	}, as.sound);
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
	output_stream << '\t' << lang::generation::minimap_icon << ' ' << mi.size.value << ' ';

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
		output_stream << ' ' << lang::generation::temp;

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

}
