#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/lang/style_overrides.hpp>
#include <fs/utility/visitor.hpp>

#include <algorithm>
#include <utility>
#include <iosfwd>
#include <optional>

namespace fs::lang
{

// needed only for some helper functions
enum class color_action_type { text, border, background };

inline int get_default_color_action_opacity(color_action_type action_type)
{
	return
		action_type == color_action_type::text   ? limits::default_set_text_color_opacity   :
		action_type == color_action_type::border ? limits::default_set_border_color_opacity :
		                                           limits::default_set_background_color_opacity;
}

struct color
{
	integer r;
	integer g;
	integer b;
	std::optional<integer> a;
};

inline bool operator==(color lhs, color rhs)
{
	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}
inline bool operator!=(color lhs, color rhs) { return !(lhs == rhs); }

struct color_action
{
	color c;
	position_tag origin;
};

inline bool operator==(color_action lhs, color_action rhs) { return lhs.c == rhs.c; }
inline bool operator!=(color_action lhs, color_action rhs) { return !(lhs == rhs); }

struct font_size_action
{
	integer size;
	position_tag origin;
};

inline bool operator==(font_size_action lhs, font_size_action rhs) { return lhs.size == rhs.size; }
inline bool operator!=(font_size_action lhs, font_size_action rhs) { return !(lhs == rhs); }

struct enabled_play_effect
{
	suit color;
	bool is_temporary;
};

inline bool operator==(enabled_play_effect lhs, enabled_play_effect rhs)
{
	return lhs.color == rhs.color && lhs.is_temporary == rhs.is_temporary;
}
inline bool operator!=(enabled_play_effect lhs, enabled_play_effect rhs) { return !(lhs == rhs); }

struct disabled_play_effect
{
	none_type none;
};

inline bool operator==(disabled_play_effect lhs, disabled_play_effect rhs) { return lhs.none == rhs.none; }
inline bool operator!=(disabled_play_effect lhs, disabled_play_effect rhs) { return !(lhs == rhs); }

struct play_effect_action
{
	std::variant<enabled_play_effect, disabled_play_effect> effect;
	position_tag origin;
};

inline bool operator==(play_effect_action lhs, play_effect_action rhs) { return lhs.effect == rhs.effect; }
inline bool operator!=(play_effect_action lhs, play_effect_action rhs) { return !(lhs == rhs); }

struct enabled_minimap_icon
{
	integer size;
	suit color;
	shape shape_;
};

inline bool operator==(enabled_minimap_icon lhs, enabled_minimap_icon rhs)
{
	return lhs.size == rhs.size && lhs.color == rhs.color && lhs.shape_ == rhs.shape_;
}
inline bool operator!=(enabled_minimap_icon lhs, enabled_minimap_icon rhs) { return !(lhs == rhs); }

struct disabled_minimap_icon
{
	position_tag sentinel_origin;
};

inline bool operator==(disabled_minimap_icon /* lhs */, disabled_minimap_icon /* rhs */) { return true; }
inline bool operator!=(disabled_minimap_icon lhs, disabled_minimap_icon rhs) { return !(lhs == rhs); }

struct minimap_icon_action
{
	static constexpr int sentinel_cancel_value = -1;

	std::variant<enabled_minimap_icon, disabled_minimap_icon> icon;
	position_tag origin;
};

inline bool operator==(minimap_icon_action lhs, minimap_icon_action rhs) { return lhs.icon == rhs.icon; }
inline bool operator!=(minimap_icon_action lhs, minimap_icon_action rhs) { return !(lhs == rhs); }

struct builtin_alert_sound_id
{
	std::variant<integer, shaper_voice_line, none_type> id;
};

inline bool operator==(builtin_alert_sound_id lhs, builtin_alert_sound_id rhs) { return lhs.id == rhs.id; }
inline bool operator!=(builtin_alert_sound_id lhs, builtin_alert_sound_id rhs) { return !(lhs == rhs); }

struct builtin_alert_sound
{
	bool is_disabled() const { return std::holds_alternative<none_type>(sound_id.id); }

	bool is_positional; // this is not a lang type because it is implied by keyword, not by value
	builtin_alert_sound_id sound_id;
};

inline bool operator==(builtin_alert_sound lhs, builtin_alert_sound rhs)
{
	return lhs.is_positional == rhs.is_positional && lhs.sound_id == rhs.sound_id;
}
inline bool operator!=(builtin_alert_sound lhs, builtin_alert_sound rhs) { return !(lhs == rhs); }

struct custom_alert_sound
{
	bool is_disabled() const { return path.value.empty() || path.value == "None"; }

	bool is_optional; // this is not a lang type because it is implied by keyword, not by value
	string path;
};

inline bool operator==(const custom_alert_sound& lhs, const custom_alert_sound& rhs)
{
	return lhs.path == rhs.path && lhs.is_optional == rhs.is_optional;
}
inline bool operator!=(const custom_alert_sound& lhs, const custom_alert_sound& rhs) { return !(lhs == rhs); }

struct alert_sound_action
{
	// Sounds are disabled with None or "None" values. In such case volume should be empty.
	bool is_disabled() const
	{
		return std::visit([](const auto& sound) { return sound.is_disabled(); }, sound);
	}

	int get_default_volume() const
	{
		return std::visit(
			utility::visitor{
				[](     builtin_alert_sound  /* sound */) { return limits::default_play_alert_sound_volume; },
				[](const custom_alert_sound& /* sound */) { return limits::default_custom_alert_sound_volume; }
			},
			sound);
	}

	std::variant<builtin_alert_sound, custom_alert_sound> sound;
	std::optional<integer> volume;
	position_tag origin;
};

inline bool operator==(const alert_sound_action& lhs, const alert_sound_action& rhs)
{
	return lhs.sound == rhs.sound && lhs.volume == rhs.volume;
}
inline bool operator!=(const alert_sound_action& lhs, const alert_sound_action& rhs) { return !(lhs == rhs); }

struct switch_drop_sound_action
{
	bool enable;
	position_tag origin;
};

inline bool operator==(switch_drop_sound_action lhs, switch_drop_sound_action rhs) { return lhs.enable == rhs.enable; }
inline bool operator!=(switch_drop_sound_action lhs, switch_drop_sound_action rhs) { return !(lhs == rhs); }

inline int override_opacity(
	color_action_type action_type,
	int nominal,
	color_overrides overrides,
	bool block_is_show,
	bool filter_is_ruthless)
{
	const int result = [&]() {
		if (!overrides.override_all_actions && action_type != color_action_type::background)
			return nominal;

		if (block_is_show) {
			return std::clamp(
				nominal,
				overrides.show_opacity_min.value_or(0),
				overrides.show_opacity_max.value_or(255));
		}
		else {
			return std::clamp(
				nominal,
				overrides.hide_opacity_min.value_or(0),
				overrides.hide_opacity_max.value_or(255));
		}
	}();

	if (filter_is_ruthless && action_type == color_action_type::text)
		return std::max(result, lang::limits::ruthless_min_set_text_color_opacity);
	else
		return result;
}

inline int override_font_size(int nominal, font_overrides overrides, bool block_is_show)
{
	if (block_is_show) {
		return std::clamp(
			nominal,
			overrides.show_size_min.value_or(limits::min_filter_font_size),
			overrides.show_size_max.value_or(limits::max_filter_font_size));
	}
	else {
		return std::clamp(
			nominal,
			overrides.hide_size_min.value_or(limits::min_filter_font_size),
			overrides.hide_size_max.value_or(limits::max_filter_font_size));
	}
}

/**
 * @class a type describing all of the given filter block actions
 *
 * all values are optional which is self-explanatory - no actions are required
 *
 * this type assumes all values (color, sizes, strings) are already verified and correct
 */
struct action_set
{
	void override_with(const action_set& other);

	// The knowledge whether a block has Show or filter is Ruthless is necessary for some overrides
	void print(std::ostream& output_stream, style_overrides overrides, bool block_is_show, bool filter_is_ruthless) const;

	std::optional<color_action> text_color;
	std::optional<color_action> border_color;
	std::optional<color_action> background_color;
	std::optional<font_size_action> font_size;
	std::optional<play_effect_action> effect;
	std::optional<minimap_icon_action> minimap_icon;
	std::optional<alert_sound_action> alert_sound;
	std::optional<switch_drop_sound_action> switch_drop_sound;
	std::optional<switch_drop_sound_action> switch_drop_sound_if_alert_sound;
};

inline bool operator==(const action_set& lhs, const action_set& rhs)
{
	return lhs.text_color == rhs.text_color
		&& lhs.border_color == rhs.border_color
		&& lhs.background_color == rhs.background_color
		&& lhs.font_size == rhs.font_size
		&& lhs.effect == rhs.effect
		&& lhs.minimap_icon == rhs.minimap_icon
		&& lhs.alert_sound == rhs.alert_sound
		&& lhs.switch_drop_sound == rhs.switch_drop_sound
		&& lhs.switch_drop_sound_if_alert_sound == rhs.switch_drop_sound_if_alert_sound;
}
inline bool operator!=(const action_set& lhs, const action_set& rhs) { return !(lhs == rhs); }

}
