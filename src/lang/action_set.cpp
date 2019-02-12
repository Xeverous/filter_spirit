#include "lang/action_set.hpp"

namespace fs::lang
{

action_set action_set::override_by(const action_set& other) const
{
	action_set result = *this;

	if (other.show)
		result.show = other.show;
	if (other.border_color)
		result.border_color = other.border_color;
	if (other.text_color)
		result.text_color = other.text_color;
	if (other.background_color)
		result.background_color = other.background_color;
	if (other.font_size)
		result.font_size = other.font_size;
	if (other.sound)
		result.sound = other.sound;
	if (other.disabled_drop_sound)
		result.disabled_drop_sound = other.disabled_drop_sound;
	if (other.icon)
		result.icon = other.icon;
	if (other.effect)
		result.effect = other.effect;

	return result;
}

}
