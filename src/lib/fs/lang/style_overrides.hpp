#pragma once

#include <fs/lang/constants.hpp>

#include <boost/optional.hpp>

namespace fs::lang {

/*
 * Design notes:
 *
 * Should overrides apply to blocks with Continue?
 * Yes. Rationale: some items may finish on Continue blocks and not applying an override there would mean that
 * some items could potentially never be affected by an override. Continue just doesn't matter, it is the
 * Show/Hide that matters (Continue is an addition to Show/Hide/Minimal, not replacement).
 *
 * Should overrides apply to blocks without the action?
 * No. Rationale: if there is no action, it means the style relies on previous block which has a Continue statement
 * which is usually paired with some actions (and such actions would be overriden there). Overriding when there
 * is no action would either result in breaking valid Continue'd styles or just needless double override.
 * If the filter has no Continue blocks one can be added at the very start that will apply all initial styles.
 *
 * Sound overrides?
 * No. Rationale: the game already has filter's overall volume slider. It is the filter writer's responsibility to
 * properly adjust volumes of individual sounds so that all sounds have similar level.
 *
 * Other overrides?
 * Nothing else makes sense to override. If more significant changes are desired, use configuration feature.
 */

struct color_overrides
{
	static constexpr auto help_opacity =
		"Clamp opacity (alpha) (A in RGBA) in generated filter's colors to be at least/at most specified value(s).\n"
		"The \"hide\" options applies to blocks with Hide in normal filters but Minimal in Ruthless filters.\n";
	boost::optional<int> show_opacity_min;
	boost::optional<int> show_opacity_max;
	boost::optional<int> hide_opacity_min;
	boost::optional<int> hide_opacity_max;

	// assert correctness of numbers in help
	static_assert(fs::lang::constants::default_set_text_color_opacity       == 255);
	static_assert(fs::lang::constants::default_set_border_color_opacity     == 255);
	static_assert(fs::lang::constants::default_set_background_color_opacity == 240);
	static_assert(fs::lang::constants::ruthless_min_set_text_color_opacity  ==  80);
	static constexpr auto help_override_all_actions =
		"If enabled, apply opacity overrides to all color actions instead of just SetBackgroundColor.\n"
		"    Note 1: Too low opacity on SetTextColor might make it unreadable.\n"
		"    Note 2: Too low opacity on SetBorderColor might make it unnoticeable.\n"
		"    Note 3: Actions with colors without opacity specified are treated as having it equal to game's defaults\n"
		"            SetBackgroundColor - 240 (slightly transparent), SetTextColor and SetBorderColor - 255 (fully opaque).\n"
		"    Note 4: Ruthless filters, inside SetTextColor must have alpha of at least 80 (game's requirement).\n"
		"            If the value is lower and this option is enabled together with Ruthless, alpha will be set to 80.";
	bool override_all_actions = false;
};

struct font_overrides
{
	static constexpr auto help_font_size =
		"Clamp font size in SetFontSize actions to be at least/at most specified value(s).\n"
		"The \"hide\" options applies to blocks with Hide in normal filters but Minimal in Ruthless filters.\n"
		"Font size changes may vary (in)significantly depending on game's font and language.";

	boost::optional<int> show_size_min;
	boost::optional<int> show_size_max;
	boost::optional<int> hide_size_min;
	boost::optional<int> hide_size_max;
};

struct style_overrides
{
	color_overrides color;
	font_overrides font;
};

}
