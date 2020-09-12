#include "user_interface/main_tab/loot_preview_state.hpp"
#include "item_preview.hpp"
#include "ui_utils.hpp"

#include <fs/utility/file.hpp>

#include <cmath>

namespace el = cycfi::elements;

namespace {

el::color to_elements_color(fs::lang::color c)
{
	return el::rgba(c.r.value, c.g.value, c.b.value, c.a.value_or(fs::lang::integer{240}).value);
}

item_preview make_item_preview(const fs::lang::item& itm, const fs::lang::item_style& style)
{
	return item_preview(
		itm.base_type,
		itm.sockets,
		itm.width,
		itm.height,
		style.font_size.size.value,
		to_elements_color(style.border_color.value_or(style.background_color).c),
		to_elements_color(style.text_color.c),
		to_elements_color(style.background_color.c));
}

} // namespace

loot_preview_state::loot_preview_state(event_inserter inserter)
: _inserter(inserter)
{
	_inserter.push_event(events::load_item_database{});
	make_ui();
}

void loot_preview_state::load_item_database(fs::log::logger& logger)
{
	std::optional<std::string> item_metadata_json = fs::utility::load_file("data/base_items.json", logger);
	if (!item_metadata_json) {
		logger.error() << "failed to load item metadata, loot preview will be disabled";
		return;
	}

	_item_database.emplace();
	if (!(*_item_database).parse(*item_metadata_json, logger)) {
		logger.error() << "failed to parse item metadata, loot preview will be disabled";
		_item_database = std::nullopt;
		return;
	}

	logger.info() << "loaded item database";
}

void loot_preview_state::make_ui()
{
	_flow_composite = el::share(el::flow_composite{});
	_main_element = el::share(el::deck(
		el::align_center_middle(el::label("load an item filter first")),
		el::htile(
			el::no_hstretch(el::vtile(
				make_button("generic currency", [this](bool) { generate_loot(loot_gen::currency_generic); }),
				make_button("divination cards", [this](bool) { generate_loot(loot_gen::divination_cards); }),
				make_button("T16 monster pack", [this](bool) { generate_loot(loot_gen::map_monster_pack); })
			)),
			el::align_top(el::flow(*_flow_composite))
		)
	));

	_root_element = el::share(make_section("loot preview", el::vtile(
		el::hold(_loot_settings.ui()),
		el::hold(_main_element)
	)));
}

void loot_preview_state::generate_loot(loot_gen gen)
{
	if (!_item_database)
		return;

	_loot.clear();

	if (gen == loot_gen::currency_generic) {
		_generator.generate_generic_currency(
			*_item_database,
			fs::lang::loot::item_inserter(_loot),
			10,
			fs::lang::loot::stack_param::single);
	}
	else if (gen == loot_gen::divination_cards) {
		_generator.generate_cards(
			*_item_database,
			fs::lang::loot::item_inserter(_loot),
			10,
			fs::lang::loot::stack_param::single);
	}
	else if (gen == loot_gen::map_monster_pack) {
		_generator.generate_monster_pack_loot(
			*_item_database,
			fs::lang::loot::item_inserter(_loot),
			_loot_settings.rarity(),
			_loot_settings.quantity(),
			83,
			true,
			50, 20, 4, 2);
	}

	_inserter.push_event(events::refresh_loot_preview{});
}

void loot_preview_state::refresh_loot_preview(const fs::lang::item_filter& filter)
{
	_loot_styles.clear();
	for (const fs::lang::item& itm : _loot) {
		_loot_styles.push_back(fs::lang::pass_item_through_filter(itm, filter, 83).style);
	}

	_flow_composite->clear();
	for (std::size_t i = 0; i < _loot.size(); ++i) {
		_flow_composite->push_back(el::share(make_item_preview(_loot[i], _loot_styles[i])));
	}

	_flow_composite->reflow();
	_main_element->select(1); // switch to loot display
}
