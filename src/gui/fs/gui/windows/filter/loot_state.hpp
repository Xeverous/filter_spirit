#pragma once

#include <fs/gui/ui_utils.hpp>
#include <fs/lang/item.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/loot/generator.hpp>
#include <fs/utility/assert.hpp>

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

// [drag] [drag] [drag] [drag] [button<str>]
class loot_button_plurality
{
public:
	loot_button_plurality(lang::loot::plurality p)
	: _min_max{p.quantity.min, p.quantity.max, p.stack_size.min, p.stack_size.max}
	{
		FS_ASSERT(min_quantity() <= max_quantity());
		FS_ASSERT(min_stack_size() <= max_stack_size());
	}

	[[nodiscard]] bool draw(const char* str); // str must not be null

	lang::loot::range quantity() const
	{
		return { min_quantity(), max_quantity() };
	}

	lang::loot::range stack_size() const
	{
		return { min_stack_size(), max_stack_size() };
	}

	lang::loot::plurality plurality() const
	{
		return { quantity(), stack_size() };
	}

	int min_quantity()   const { return _min_max[0]; }
	int max_quantity()   const { return _min_max[1]; }
	int min_stack_size() const { return _min_max[2]; }
	int max_stack_size() const { return _min_max[3]; }

private:
	int& min_quantity()   { return _min_max[0]; }
	int& max_quantity()   { return _min_max[1]; }
	int& min_stack_size() { return _min_max[2]; }
	int& max_stack_size() { return _min_max[3]; }

	int _min_max[4];
};

class loot_widget_range_base
{
public:
	loot_widget_range_base(lang::loot::range r)
	: _min_max{r.min, r.max}
	{
		FS_ASSERT(min() <= max());
	}

	lang::loot::range range() const
	{
		return { min(), max() };
	}

	int min() const { return _min_max[0]; }
	int max() const { return _min_max[1]; }

protected:
	int& min() { return _min_max[0]; }
	int& max() { return _min_max[1]; }

	int _min_max[2];
};

// [drag] [drag] [button<str>]
class loot_button_drag_range : public loot_widget_range_base
{
public:
	loot_button_drag_range(lang::loot::range r)
	: loot_widget_range_base(r)
	{
	}

	[[nodiscard]] bool draw(const char* str); // str must not be null
};

// [drag] [drag] [label<str>]
class loot_drag_range : public loot_widget_range_base
{
public:
	loot_drag_range(lang::loot::range current)
	: loot_widget_range_base(current)
	{
	}

	void draw(const char* str); // str must not be null
};

// [slider] [slider] [label<str>]
class loot_slider_range : public loot_widget_range_base
{
public:
	loot_slider_range(lang::loot::range current, lang::loot::range limit)
	: loot_widget_range_base(current)
	, _limit(limit)
	{
	}

	loot_slider_range(lang::loot::range current)
	: loot_widget_range_base(current)
	, _limit(current)
	{
	}

	void draw(const char* str); // str must not be null

private:
	lang::loot::range _limit;
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
		_last_items_size = 0;
		_canvas_offset_y = 0.0f;
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

	std::size_t num_items() const
	{
		return _items.size();
	}

private:
	void draw_loot_canvas(const fonting& f);
	void draw_item_labels(ImVec2 canvas_begin, ImVec2 canvas_end, const fonting& f);
	void on_canvas_hover(ImVec2 mouse_position, const fonting& f);

	void draw_loot_settings_global();
	void draw_loot_buttons_currency        (const lang::loot::item_database& db, lang::loot::generator& gen);
	void draw_loot_buttons_specific_classes(const lang::loot::item_database& db, lang::loot::generator& gen);
	void draw_loot_buttons_gems            (const lang::loot::item_database& db, lang::loot::generator& gen);
	void draw_loot_buttons_equipment       (const lang::loot::item_database& db, lang::loot::generator& gen);

	// render state
	float _canvas_offset_y = 0;

	// loot state
	std::vector<looted_item> _items;
	std::size_t _last_items_size = 0;

	// global loot settings
	bool _append_loot = false;
	bool _shuffle_loot = false;
	int _area_level = 83;
	int _player_iir = 0;
	int _map_iir = 0;
	lang::loot::percent _chance_to_identify;
	lang::loot::percent _chance_to_corrupt;
	lang::loot::percent _chance_to_mirror;

	// currency
	loot_button_plurality _currency_generic           = loot_button_plurality(lang::loot::plurality::only_quantity({1, 10}));
	loot_button_plurality _currency_generic_shards    = loot_button_plurality(lang::loot::plurality{{1, 5}, {1, 19}});
	loot_button_plurality _currency_conqueror_orbs    = loot_button_plurality(lang::loot::plurality::only_quantity({1,  2}));
	loot_button_plurality _currency_breach_blessings  = loot_button_plurality(lang::loot::plurality::only_quantity({1,  2}));
	loot_button_plurality _currency_breach_splinters  = loot_button_plurality(lang::loot::plurality::only_quantity({1, 10}));
	loot_button_plurality _currency_legion_splinters  = loot_button_plurality(lang::loot::plurality::only_quantity({1, 10}));
	loot_button_plurality _currency_essences          = loot_button_plurality(lang::loot::plurality::only_quantity({2,  5}));
	loot_button_plurality _currency_fossils           = loot_button_plurality(lang::loot::plurality::only_quantity({1,  5}));
	loot_button_plurality _currency_catalysts         = loot_button_plurality(lang::loot::plurality{{2, 5}, {1, 9}});
	loot_button_plurality _currency_oils              = loot_button_plurality(lang::loot::plurality::only_quantity({3, 10}));
	loot_button_plurality _currency_delirium_orbs     = loot_button_plurality(lang::loot::plurality::only_quantity({1,  3}));
	loot_button_plurality _currency_harbinger_scrolls = loot_button_plurality(lang::loot::plurality::only_quantity({1,  2}));
	loot_button_plurality _currency_incursion_vials   = loot_button_plurality(lang::loot::plurality::only_quantity({1,  1}));
	loot_button_plurality _currency_bestiary_nets     = loot_button_plurality(lang::loot::plurality{{1, 2}, {1, 9}});

	// specific classes
	loot_button_plurality  _class_cards               = loot_button_plurality(lang::loot::plurality::only_quantity({1, 10}));
	loot_button_plurality  _class_resonators          = loot_button_plurality(lang::loot::plurality{{1, 5}, {1, 2}});
	loot_button_drag_range _class_incubators          = loot_button_drag_range({2, 5});
	loot_button_drag_range _class_metamorphs          = loot_button_drag_range({1, 2});
	loot_button_drag_range _class_unique_pieces       = loot_button_drag_range({1, 2});
	loot_button_drag_range _class_lab_keys            = loot_button_drag_range({1, 2});
	loot_button_drag_range _class_lab_trinkets        = loot_button_drag_range({1, 1});
	loot_button_drag_range _class_quest_items         = loot_button_drag_range({1, 2});

	// gems
	lang::loot::gem_types _gems_gem_types;
	loot_drag_range   _gems_quantity                  = loot_drag_range({1, 10});
	loot_slider_range _gems_level                     = loot_slider_range({1, 20});
	loot_slider_range _gems_quality                   = loot_slider_range({0, 20});

	// equipment - main
	lang::loot::equippable_item_weights _eq_class_weights;
	lang::loot::influence_weights       _eq_influence_weights;
	loot_slider_range                   _eq_quality  = loot_slider_range({0, 20});
	// equipment - fixed weights generation
	loot_drag_range                _eq_fixed_weights_quantity = loot_drag_range({25, 40});
	lang::loot::rarity_weights     _eq_fixed_weights_rarity;
	lang::loot::item_level_weights _eq_fixed_weights_item_level;
	// equipment - monster pack generation
	loot_drag_range                _eq_monster_pack_normal = loot_drag_range({30, 50});
	loot_drag_range                _eq_monster_pack_magic  = loot_drag_range({ 5, 15});
	loot_drag_range                _eq_monster_pack_rare   = loot_drag_range({ 1,  4});
	loot_drag_range                _eq_monster_pack_unique = loot_drag_range({ 1,  2});
};

}
