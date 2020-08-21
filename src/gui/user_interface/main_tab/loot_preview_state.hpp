#pragma once

#include "event.hpp"
#include "user_interface/main_tab/loot_preview/loot_preview_settings_state.hpp"

#include <fs/lang/loot/item_database.hpp>
#include <fs/lang/loot/generator.hpp>
#include <fs/lang/item.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/log/logger.hpp>

#include <elements/element.hpp>

#include <optional>

class loot_preview_state
{
public:
	loot_preview_state(event_inserter inserter);

	void load_item_database(fs::log::logger& logger);


	void refresh_loot_preview(const fs::lang::item_filter& filter);

	std::shared_ptr<cycfi::elements::element> ui()
	{
		return _root_element;
	}

private:
	void make_ui();

	enum class loot_gen {
		currency_generic,
		divination_cards,
		map_monster_pack
	};

	void generate_loot(loot_gen gen);

	event_inserter _inserter;

	std::optional<fs::lang::loot::item_database> _item_database;
	fs::lang::loot::generator _generator;

	std::vector<fs::lang::item> _loot;
	std::vector<fs::lang::item_style> _loot_styles;

	// UI
	std::shared_ptr<cycfi::elements::element> _root_element;
	std::shared_ptr<cycfi::elements::flow_composite> _flow_composite;
	std::shared_ptr<cycfi::elements::deck_element> _main_element;
	loot_preview_settings_state _loot_settings;
};
