#pragma once

#include <fs/gui/windows/filter_windows_fwd.hpp>
#include <fs/gui/windows/filter/real_filter_state_mediator.hpp>
#include <fs/gui/windows/filter/spirit_filter_state_mediator.hpp>

#include <string>

namespace fs::gui {

class real_filter_window : public filter_window
{
public:
#ifndef __EMSCRIPTEN__
	static real_filter_window from_file(std::string path);
#endif
	static real_filter_window from_text_input();
	static real_filter_window from_source(std::string name, std::string source);

	void draw_impl(
		const gui_settings& settings,
		const lang::loot::item_database& db,
		lang::loot::generator& gen,
		network::cache& network_cache) override;

private:
	real_filter_window();

	real_filter_state_mediator _state;
};

class spirit_filter_window : public filter_window
{
public:
#ifndef __EMSCRIPTEN__
	static spirit_filter_window from_file(const network::cache& network_cache, std::string path);
#endif
	static spirit_filter_window from_text_input(const network::cache& network_cache);
	static spirit_filter_window from_source(const network::cache& network_cache, std::string name, std::string source);

	void draw_impl(
		const gui_settings& settings,
		const lang::loot::item_database& db,
		lang::loot::generator& gen,
		network::cache& network_cache) override;

private:
	spirit_filter_window(const network::cache& network_cache);

	spirit_filter_state_mediator _state;
};

}
