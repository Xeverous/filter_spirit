#pragma once

#include <fs/gui/windows/filter_windows_fwd.hpp>
#include <fs/gui/windows/color_picker_window.hpp>
#include <fs/gui/windows/single_item_preview_window.hpp>
#include <fs/gui/windows/settings_window.hpp>
#include <fs/gui/windows/about_window.hpp>
#include <fs/gui/windows/version_info_window.hpp>
#include <fs/gui/windows/application_log_window.hpp>
#include <fs/gui/auxiliary/file_dialogs.hpp>
#include <fs/lang/loot/item_database.hpp>
#include <fs/lang/loot/generator.hpp>
#include <fs/network/item_price_report.hpp>

#include <Magnum/ImGuiIntegration/Context.hpp>

#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#include <Magnum/Platform/EmscriptenApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

#include <vector>
#include <memory>

namespace fs::gui {

class application: public Magnum::Platform::Application
{
public:
	explicit application(const Arguments& arguments);

	void drawEvent() override;

	/*
	 * This would normally be "tickEvent() override"
	 * but such function is not supported on Emscripten,
	 * so it is called in drawEvent instead.
	 */
	void tick_event();

	void viewportEvent(ViewportEvent& event) override;

	void keyPressEvent(KeyEvent& event) override;
	void keyReleaseEvent(KeyEvent& event) override;

	void mousePressEvent(MouseEvent& event) override;
	void mouseReleaseEvent(MouseEvent& event) override;
	void mouseMoveEvent(MouseMoveEvent& event) override;
	void mouseScrollEvent(MouseScrollEvent& event) override;
	void textInputEvent(TextInputEvent& event) override;

private:
	void draw_main_menu_bar();

	void remove_closed_windows();
	void process_open_file_modals();
	void rebuild_pending_fonts();

	Magnum::ImGuiIntegration::Context _imgui{Magnum::NoCreate};

	// Application-global logger instance.
	// If errors appear in it, something went horribly wrong.
	application_log_window _application_log;

	// Holds a lot of data that is not being changed during lifetime of the application.
	// This should be the only (application-global) instance for use by filter/loot debugger.
	lang::loot::item_database _item_database;
	lang::loot::generator _loot_generator;

	network::cache _network_cache;

	bool _show_demo_window = false;
	bool _force_focus_demo_window = false;

	enum class modal_dialog_state_type
	{
		none,
		open_spirit_filter,
		open_real_filter
	};

	modal_dialog_state_type _modal_dialog_state = modal_dialog_state_type::none;
	aux::open_file_dialog _open_file_dialog;

	std::vector<std::unique_ptr<filter_window>> _filters;

	color_picker_window _color_picker;
	single_item_preview_window _single_item_preview;
	settings_window _settings;
	about_window _about;
	version_info_window _version_info;
};

}
