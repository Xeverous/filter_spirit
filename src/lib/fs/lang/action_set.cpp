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
	output_stream << '\t' << name << ' ' << c.r << ' ' << c.g << ' ' << c.b;
	if (c.a.has_value())
		output_stream << ' ' << *c.a;

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

void output_built_in_alert_sound(
	lang::built_in_alert_sound alert_sound,
	std::ostream& output_stream)
{
	output_stream << '\t';

	if (alert_sound.is_positional.value)
		output_stream << kw::play_alert_sound_positional;
	else
		output_stream << kw::play_alert_sound;

	output_stream << ' ' << alert_sound.id.value;

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
		[&output_stream](lang::built_in_alert_sound sound) {
			output_built_in_alert_sound(sound, output_stream);
		},
		[&output_stream](const lang::custom_alert_sound& sound) {
			output_custom_alert_sound(sound, output_stream);
		}
	}, as.sound);
}

void output_disabled_drop_sound(
	std::optional<lang::disable_default_drop_sound_action> action,
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
		default:
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
		default:
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
	std::optional<lang::beam_effect_action> beam_effect_action,
	std::ostream& output_stream)
{
	if (!beam_effect_action.has_value())
		return;

	const lang::beam_effect& be = (*beam_effect_action).beam;
	output_stream << '\t' << kw::play_effect << ' ';
	output_suit(be.color, output_stream);
	if (be.is_temporary.value)
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
	output_alert_sound(set_alert_sound, output_stream);
	output_disabled_drop_sound(disable_default_drop_sound, output_stream);
	output_minimap_icon(set_minimap_icon, output_stream);
	output_beam_effect(set_beam_effect, output_stream);
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

	if (other.set_alert_sound)
		set_alert_sound = std::move(*other.set_alert_sound);

	disable_default_drop_sound = other.disable_default_drop_sound;

	if (other.set_minimap_icon)
		set_minimap_icon = *other.set_minimap_icon;

	if (other.set_beam_effect)
		set_beam_effect = *other.set_beam_effect;
}

bool operator==(const action_set& lhs, const action_set& rhs)
{
	return lhs.set_border_color == rhs.set_border_color
		&& lhs.set_text_color == rhs.set_text_color
		&& lhs.set_background_color == rhs.set_background_color
		&& lhs.set_font_size == rhs.set_font_size
		&& lhs.set_alert_sound == rhs.set_alert_sound
		&& lhs.disable_default_drop_sound == rhs.disable_default_drop_sound
		&& lhs.set_minimap_icon == rhs.set_minimap_icon
		&& lhs.set_beam_effect == rhs.set_beam_effect;
}

bool operator!=(const action_set& lhs, const action_set& rhs)
{
	return !(lhs == rhs);
}

}
