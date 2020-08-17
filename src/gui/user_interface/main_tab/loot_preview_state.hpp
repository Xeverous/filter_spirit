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

	std::shared_ptr<cycfi::elements::element> make_ui();
	void load_item_database(fs::log::logger& logger);

	void generate_loot_divination_cards();
	void generate_loot_currency_generic();

	void refresh_loot_preview(const fs::lang::item_filter& filter);

private:
	event_inserter _inserter;

	std::optional<fs::lang::loot::item_database> _item_database;
	fs::lang::loot::generator _generator;

	std::vector<fs::lang::item> _loot;
	std::vector<fs::lang::item_style> _loot_styles;

	// UI
	std::shared_ptr<cycfi::elements::flow_composite> _flow_composite;
	std::shared_ptr<cycfi::elements::deck_element> _main_element;
	loot_preview_settings_state _loot_settings;
};
