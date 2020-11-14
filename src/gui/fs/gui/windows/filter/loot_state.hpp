#pragma once

#include <fs/lang/item.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/loot/generator.hpp>

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
};

class application;

class loot_state : public lang::loot::item_receiver
{
public:
	void draw_interface(application& app);

	void refilter_items(const lang::item_filter& filter);

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
		_items.push_back(looted_item{itm, std::nullopt});
	}

	void on_item(lang::item&& itm) override
	{
		_items.push_back(looted_item{std::move(itm), std::nullopt});
	}

private:
	int _area_level = 1;
	std::vector<looted_item> _items;
	std::optional<std::size_t> _selected_item_index;
};

}
