#pragma once

#include <fs/gui/ui_utils.hpp>
#include <fs/lang/item.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/loot/generator.hpp>

#include <imgui.h>

#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <functional>

namespace fs::gui {

struct looted_item
{
	lang::item itm;
	std::optional<lang::item_filtering_result> filtering_result;
	std::optional<rect> drawing; // in screen coordinates
};

class application;
class fonting;

class loot_state : public lang::loot::item_receiver
{
public:
	void draw_interface(application& app);

	void update_items(const lang::item_filter& filter); // apply filter to items without style
	void refilter_items(const lang::item_filter& filter); // apply filter to every item

	void clear_items()
	{
		_items.clear();
	}

	void clear_filter_results()
	{
		for (looted_item& itm : _items)
			itm.filtering_result = std::nullopt;
	}

	void on_item(const lang::item& itm) override
	{
		_items.push_back(looted_item{itm, std::nullopt, std::nullopt});
	}

	void on_item(lang::item&& itm) override
	{
		_items.push_back(looted_item{std::move(itm), std::nullopt, std::nullopt});
	}

private:
	void draw_loot_canvas(const fonting& f);
	void draw_item_labels(ImVec2 canvas_begin, ImVec2 canvas_end, const fonting& f);
	void on_canvas_hover(ImVec2 mouse_position, const fonting& f);

	void draw_loot_settings_global();

	bool _append_loot = false;
	bool _shuffle_loot = false;
	int _area_level = 83;
	int _player_iiq = 0;
	int _player_iir = 0;
	int _map_iiq = 0;
	int _map_iir = 0;
	std::vector<looted_item> _items;

	float _canvas_offset_y = 0;
};

}
