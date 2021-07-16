#pragma once

#include <fs/gui/imgui_window.hpp>

#include <memory>
#include <string>

namespace fs::lang::loot {

struct item_database;
class generator;

}

namespace fs::network { struct cache; }

namespace fs::gui {

constexpr auto str_real_filter_from_text_input = "Real filter - from text input";
constexpr auto str_spirit_filter_from_text_input = "Spirit filter - from text input";

struct gui_settings;

class filter_window : public imgui_window
{
public:
	filter_window();

	virtual ~filter_window() = default;

	void draw(
		const gui_settings& settings,
		const lang::loot::item_database& db,
		lang::loot::generator& loot_generator,
		network::cache& network_cache)
	{
		draw_window([&, this]() {
			draw_impl(settings, db, loot_generator, network_cache);
		});
	}

protected:
	virtual void draw_impl(
		const gui_settings& settings,
		const lang::loot::item_database& db,
		lang::loot::generator& loot_generator,
		network::cache& network_cache) = 0;
};

#ifndef __EMSCRIPTEN__
std::unique_ptr<filter_window> real_filter_window_from_file(std::string path);
std::unique_ptr<filter_window> spirit_filter_window_from_file(const network::cache& network_cache, std::string path);
#endif

std::unique_ptr<filter_window> real_filter_window_from_text_input();
std::unique_ptr<filter_window> spirit_filter_window_from_text_input(const network::cache& network_cache);
std::unique_ptr<filter_window> real_filter_window_from_source(std::string name, std::string source);
std::unique_ptr<filter_window> spirit_filter_window_from_source(const network::cache& network_cache, std::string name, std::string source);

}
