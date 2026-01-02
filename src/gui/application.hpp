#pragma once

#include "ui_state.hpp"

#include <fs/network/item_price_report.hpp>
#include <fs/lang/loot/generator.hpp>
#include <fs/lang/loot/item_database.hpp>

#include <nukleus.hpp>

namespace fs::gui {

class application
{
public:
	void draw(nk::context& ctx, ui_state& uis);

private:
	//void process_open_file_modals();
	//void rebuild_pending_fonts();

	// Application-global logger instance.
	// If errors appear in it, something went horribly wrong.
	//application_log_window m_application_log;

	// Holds a lot of data that is not being changed during lifetime of the application.
	// This should be the only (application-global) instance for use by filter/loot debugger.
	lang::loot::item_database m_item_database;
	lang::loot::generator m_loot_generator;

	network::cache m_network_cache;
	//aux::open_file_dialog m_open_file_dialog;

	//std::vector<std::unique_ptr<filter_window>> m_filters;

	//color_picker_window m_color_picker;
	//single_item_preview_window m_single_item_preview;
	//settings_window m_settings;
	//about_window m_about;
	//version_info_window m_version_info;
};

}
