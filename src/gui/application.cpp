#include "application.hpp"

namespace fs::gui {

namespace {

bool draw_menu(nk::context& ctx, ui_state& uis)
{
	auto win = ctx.window_scoped("menubar", {0.0f, 0.0f, static_cast<float>(uis.window_width), ctx.get_current_font_height() * 4.0f}, nk::window_flags::none);
	auto menubar = win.menubar_scoped();
	auto row = win.layout_row_static_scoped(0.0f, 3);

	row.push(ctx.get_text_width("File") + uis.font_size);
	if (auto menu = win.menu_label_scoped("File", {200, 200}))
	{
		win.layout_row_dynamic(0.0f, 1);
		if (menu.item_label("Open *.filter ...")) {

		}
		if (menu.item_label("Open *.ruthlessfilter ...")) {

		}
		if (menu.item_label("Open *.filtertemplate ...")) {

		}

		win.layout_row_dynamic(2, 1);
		win.rule_horizontal(nk::color::from_rgb(0, 0, 0), false);

		win.layout_row_dynamic(0.0f, 1);
		if (menu.item_label("Exit"))
			return false;
	}

	row.push(ctx.get_text_width("Settings") + uis.font_size);
	if (auto menu = win.menu_label_scoped("Settings", {200, 600}))
	{
		win.layout_row_dynamic(0.0f, 1);
		auto disable = win.widget_disable_scoped();
		win.label("(nothing)");
	}

	row.push(ctx.get_text_width("Help") + uis.font_size);
	if (auto menu = win.menu_label_scoped("Help", {120, 200}))
	{
		win.layout_row_dynamic(0.0f, 1);
		if (menu.item_label("About")) {

		}
	}

	return true;
}

bool draw_font_settings_window(nk::context& ctx, ui_state& uis)
{
	auto win = ctx.window_scoped("font settings", {200.0f, 200.0f, 600.0f, 600.0f});
	if (!win)
		return false;

	win.layout_row_dynamic(uis.font_size * 2, 1);

	const float current_font_size = uis.font_size;
	bool result = false;
	win.property_in_place("font size", 4.0f, uis.font_size, 72.0f, 0.0625f, 0.0625f);

	win.layout_row_static(uis.font_size * 2, uis.font_size * 4, 2);
	if (win.button_label("-1"))
		uis.font_size -= 1.0f;
	if (win.button_label("+1"))
		uis.font_size += 1.0f;
	if (win.button_label("-1/2"))
		uis.font_size -= 0.5f;
	if (win.button_label("+1/2"))
		uis.font_size += 0.5f;
	if (win.button_label("-1/4"))
		uis.font_size -= 0.25f;
	if (win.button_label("+1/4"))
		uis.font_size += 0.25f;
	if (win.button_label("-1/8"))
		uis.font_size -= 0.125f;
	if (win.button_label("+1/8"))
		uis.font_size += 0.125f;
	if (win.button_label("-1/16"))
		uis.font_size -= 0.0625f;
	if (win.button_label("+1/16"))
		uis.font_size += 0.0625f;

	if (uis.font_size != current_font_size)
		result = true;

	win.layout_row_dynamic(uis.font_size, 1);
	win.label("ASCII only test string... @ # ! $");

	return result;
}

}

void application::draw(nk::context& ctx, ui_state& uis)
{
	draw_menu(ctx, uis);

	if (draw_font_settings_window(ctx, uis))
	{
		uis.regenerate_font_atlas();
		ctx.style_set_font(uis.fontin_regular->handle);
	}
}

}
