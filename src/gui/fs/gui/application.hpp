#pragma once

#include <fs/gui/windows/filter_windows.hpp>
#include <fs/gui/windows/color_picker_window.hpp>
#include <fs/gui/windows/single_item_preview_window.hpp>
#include <fs/gui/windows/common_ui_settings_window.hpp>
#include <fs/gui/windows/version_info_window.hpp>
#include <fs/gui/windows/application_log_window.hpp>
#include <fs/gui/settings/theming.hpp>
#include <fs/gui/settings/fonting.hpp>
#include <fs/lang/loot/item_database.hpp>
#include <fs/lang/loot/generator.hpp>

#include <Magnum/ImGuiIntegration/Context.hpp>

#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#include <Magnum/Platform/EmscriptenApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

#include <vector>
#include <optional>

namespace fs::gui {

class application: public Magnum::Platform::Application
{
public:
	explicit application(const Arguments& arguments);

	void drawEvent() override;
	void tickEvent() override;

	void viewportEvent(ViewportEvent& event) override;

	void keyPressEvent(KeyEvent& event) override;
	void keyReleaseEvent(KeyEvent& event) override;

	void mousePressEvent(MouseEvent& event) override;
	void mouseReleaseEvent(MouseEvent& event) override;
	void mouseMoveEvent(MouseMoveEvent& event) override;
	void mouseScrollEvent(MouseScrollEvent& event) override;
	void textInputEvent(TextInputEvent& event) override;

	fonting& font_settings()
	{
		return _fonting;
	}

	theming& theme_settings()
	{
		return _theming;
	}

	auto& item_database()
	{
		return _item_database;
	}

	auto& loot_generator()
	{
		return _loot_generator;
	}

private:
	void draw_main_menu_bar();

	void on_open_spirit_filter();
	void on_open_real_filter();

	void remove_closed_windows();
	void open_pending_modals();
	void rebuild_pending_fonts();

	Magnum::ImGuiIntegration::Context _imgui{Magnum::NoCreate};

	theming _theming;
	fonting _fonting;

	// Application-global logger instance.
	// If errors appear in it, something went horribly wrong.
	application_log_window _application_log;

	// Holds a lot of data that is not being changed during lifetime of the application.
	// This should be the only (application-global) instance for use by filter/loot debugger.
	std::optional<lang::loot::item_database> _item_database;
	lang::loot::generator _loot_generator;

	bool _show_demo_window = false;
	bool _force_focus_demo_window = false;

	enum class modal_dialog_state_type
	{
		none,
		open_spirit_filter,
		open_real_filter
	};

	modal_dialog_state_type _modal_dialog_state = modal_dialog_state_type::none;
	std::vector<spirit_filter_window> _spirit_filters;
	std::vector<real_filter_window> _real_filters;

	color_picker_window _color_picker;
	single_item_preview_window _single_item_preview;
	common_ui_settings_window _common_ui_settings;
	version_info_window _version_info;
};

}
