#include <fs/gui/windows/filter/loot_state.hpp>
#include <fs/gui/application.hpp>

#include <imgui.h>

namespace fs::gui {

void loot_state::draw_interface(application& app)
{
	std::optional<lang::loot::item_database>& database = app.item_database();
	if (!database) {
		ImGui::TextWrapped("Item database failed to load. Loot generation not available.");
		return;
	}

	lang::loot::item_database& db = *database;

	if (ImGui::Button("random currency")) {
		app.loot_generator().generate_generic_currency(db, *this, 10, fs::lang::loot::stack_param::single);
	}
}

void loot_state::refilter_items(const lang::item_filter& filter)
{
	for (looted_item& itm : _items) {
		itm.filtering_result = lang::pass_item_through_filter(itm.itm, filter, _area_level);
	}
}

}
