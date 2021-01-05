#include <fs/gui/windows/filter/loot_state.hpp>
#include <fs/gui/windows/filter/item_tooltip.hpp>
#include <fs/gui/windows/filter/filter_state_mediator.hpp>
#include <fs/gui/application.hpp>
#include <fs/gui/settings/fonting.hpp>
#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/gui/auxiliary/raii.hpp>
#include <fs/gui/auxiliary/colors.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/lang/limits.hpp>
#include <fs/utility/assert.hpp>

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cmath>

namespace {

using namespace fs;

class item_label_data
{
public:
	item_label_data(const lang::item& itm, int font_size, const gui::fonting& f)
	{
		if (itm.name)
			_first_line_size = gui::aux::measure_text_line(*itm.name, font_size, f.filter_preview_font(font_size));

		const int sz = lang::snprintf_dropped_item_label(_second_line_buf.data(), _second_line_buf.size(), itm);
		FS_ASSERT_MSG(sz >= 0, "snprintf should not return an error");
		(void) sz; // shut warning if assert does not use variable
		_second_line_size = gui::aux::measure_text_line(_second_line_buf.data(), font_size, f.filter_preview_font(font_size));

		_whole_size = {
			std::max(_first_line_size.x, _second_line_size.x) + 2.0f * padding_x,
			has_first_line() ?
				_first_line_size.y + _second_line_size.y + 3.0f * padding_y :
				_second_line_size.y + 2.0f * padding_y
		};
	}

	ImVec2 whole_size() const
	{
		return _whole_size;
	}

	bool has_first_line() const
	{
		return _first_line_size.x != 0.0f && _first_line_size.y != 0.0f;
	}

	ImVec2 first_line_size() const
	{
		return _first_line_size;
	}

	ImVec2 second_line_size() const
	{
		return _second_line_size;
	}

	const char* second_line() const
	{
		return _second_line_buf.data();
	}

	// use floats instead of Dear ImGui types because the library has no constexpr support
	static constexpr auto padding_x = 20.0f;
	static constexpr auto padding_y = 5.0f;

private:
	std::array<char, 104> _second_line_buf;
	ImVec2 _first_line_size;
	ImVec2 _second_line_size;
	ImVec2 _whole_size;
};

ImU32 to_imgui_color(lang::color c)
{
	if (c.a)
		return IM_COL32(c.r.value, c.g.value, c.b.value, (*c.a).value);
	else
		return IM_COL32(c.r.value, c.g.value, c.b.value, lang::limits::default_filter_opacity);
}

ImU32 to_imgui_color(lang::socket_color color)
{
	if (color == lang::socket_color::r)
		return IM_COL32(202,  13,  50, 255);
	else if (color == lang::socket_color::g)
		return IM_COL32(158, 203,  13, 255);
	else if (color == lang::socket_color::b)
		return IM_COL32( 88, 130, 254, 255);
	else if (color == lang::socket_color::w)
		return IM_COL32(200, 200, 200, 255);
	else if (color == lang::socket_color::a)
		return IM_COL32( 59,  59,  59, 255);
	else if (color == lang::socket_color::d)
		return IM_COL32(  0,   0,   0,   0); // resonators do not render sockets on ground labels

	FS_ASSERT(false);
	return IM_COL32(0, 0, 0, 255);
}

void draw_item_sockets(gui::aux::rect r, const lang::item& itm)
{
	const int num_sockets = itm.sockets.sockets();

	if (num_sockets == 0)
		return;

	FS_ASSERT(num_sockets <= 6);

	const ImVec2 link_horizontal_size(3.0f, 2.0f);
	const ImVec2 link_vertical_size(2.0f, 3.0f);
	const ImVec2 socket_size(4.0f, 4.0f);
	ImDrawList* const draw_list = ImGui::GetWindowDrawList();
	const ImU32 link_color = IM_COL32(195, 195, 195, 255);

	if (itm.width == 1 || num_sockets == 1) { // vertical drawing
		// floor calls are added to perform pixel-perfect drawing
		const float socket_begin_x = std::floor(r.top_left.x + (r.size.x - socket_size.x) / 2.0f);
		      float socket_begin_y = std::floor(r.top_left.y +
			(r.size.y - num_sockets * socket_size.y - (num_sockets - 1) * link_vertical_size.y) / 2.0f);

		lang::traverse_sockets(
			itm.sockets,
			[&](lang::socket_color color) {
				if (color != lang::socket_color::d) {
					draw_list->AddRectFilled(
						{socket_begin_x                , socket_begin_y                },
						{socket_begin_x + socket_size.x, socket_begin_y + socket_size.y},
						to_imgui_color(color));
				}
				socket_begin_y += socket_size.y;
			},
			[&](bool link) {
				if (link) {
					draw_list->AddRectFilled(
						{socket_begin_x + 1.0f                       , socket_begin_y                       },
						{socket_begin_x + 1.0f + link_vertical_size.x, socket_begin_y + link_vertical_size.y},
						link_color);
				}
				socket_begin_y += link_vertical_size.y;
			});
	}
	else { // drawing in "2" shape
		FS_ASSERT(num_sockets >= 2);
		/*
		 * 0 - 1
		 *     |
		 * 3 - 2
		 * |
		 * 4 - 5
		 */
		const ImVec2 socket_offsets[] = {
			/* 0 */ {                                  0.0f,                                         0.0f},
			/* 1 */ {socket_size.x + link_horizontal_size.x,                                         0.0f},
			/* 2 */ {socket_size.x + link_horizontal_size.x,         socket_size.y + link_vertical_size.y},
			/* 3 */ {                                  0.0f,         socket_size.y + link_vertical_size.y},
			/* 4 */ {                                  0.0f, 2.0f * (socket_size.y + link_vertical_size.y)},
			/* 5 */ {socket_size.x + link_horizontal_size.x, 2.0f * (socket_size.y + link_vertical_size.y)}
		};
		const ImVec2 link_offsets[] = {
			/* 0 */ {socket_size.x                                ,                                                 1.0f},
			/* 1 */ {socket_size.x + link_horizontal_size.x + 1.0f,         socket_size.y                               },
			/* 2 */ {socket_size.x                                ,         socket_size.y + link_vertical_size.y  + 1.0f},
			/* 3 */ {                                         1.0f, 2.0f *  socket_size.y + link_vertical_size.y        },
			/* 4 */ {socket_size.x                                , 2.0f * (socket_size.y + link_vertical_size.y) + 1.0f},
		};

		const int num_rows = (num_sockets + 1) / 2;

		const float begin_x = std::floor(r.top_left.x +
			(r.size.x - 2.0f * socket_size.x - link_horizontal_size.x) / 2.0f);
		const float begin_y = std::floor(r.top_left.y +
			(r.size.y - num_rows * socket_size.y - (num_rows - 1) * link_vertical_size.y) / 2.0f);

		int index = 0;
		lang::traverse_sockets(
			itm.sockets,
			[&](lang::socket_color color) {
				if (color == lang::socket_color::d)
					return;

				const auto x = begin_x + socket_offsets[index].x;
				const auto y = begin_y + socket_offsets[index].y;
				draw_list->AddRectFilled({x, y}, {x + socket_size.x, y + socket_size.y}, to_imgui_color(color));
			},
			[&](bool link) {
				if (link) {
					const auto x = begin_x + link_offsets[index].x;
					const auto y = begin_y + link_offsets[index].y;

					const bool is_horizontal = index % 2 == 0;
					const ImVec2 size = is_horizontal ? link_horizontal_size : link_vertical_size;

					draw_list->AddRectFilled({x, y}, {x + size.x, y + size.y}, link_color);
				}
				++index;
			});
	}
}

void draw_item_label(
	ImVec2 item_begin,
	const item_label_data& ild,
	const lang::item& itm,
	const lang::item_filtering_result& result,
	const gui::fonting& f)
{
	const ImVec2 item_size = ild.whole_size();
	const ImVec2 item_end(item_begin.x + item_size.x, item_begin.y + item_size.y);

	ImDrawList* const draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(item_begin, item_end, to_imgui_color(result.style.background_color.c));

	if (result.style.border_color)
		draw_list->AddRect(item_begin, item_end, to_imgui_color((*result.style.border_color).c));

	const auto font_size = result.style.font_size.size.value;
	const auto fnt = f.filter_preview_font(font_size);

	auto y = item_begin.y;

	if (itm.name) {
		const auto x = item_begin.x + (item_size.x - ild.first_line_size().x) / 2.0f;
		draw_list->AddText(fnt, font_size, ImVec2(x, y), to_imgui_color(result.style.text_color.c), (*itm.name).c_str());
		y += item_label_data::padding_y + font_size;
	}

	const auto x = item_begin.x + (item_size.x - ild.second_line_size().x) / 2.0f;
	draw_list->AddText(fnt, font_size, ImVec2(x, y), to_imgui_color(result.style.text_color.c), ild.second_line());

	draw_item_sockets(
		gui::aux::rect{
			ImVec2(item_end.x - item_label_data::padding_x, item_begin.y),
			ImVec2(item_label_data::padding_x, item_size.y)},
		itm
	);
}

void draw_percent_slider(const char* str, lang::loot::percent& value)
{
	ImGui::SliderInt(str, &value.value, lang::loot::percent::min().value, lang::loot::percent::max().value, "%d%%", ImGuiSliderFlags_AlwaysClamp);
}

void draw_weight_drag(const char* str, int& value)
{
	gui::aux::scoped_pointer_id _(&value);
	ImGui::DragInt(str, &value, 1.0f, 0, INT_MAX, "%d", ImGuiSliderFlags_AlwaysClamp);
}

const gui::looted_item* find_item_by_mouse_position(const std::vector<gui::looted_item>& items, ImVec2 mouse_position)
{
	for (const auto& itm : items) {
		if (itm.drawing && (*itm.drawing).contains(mouse_position))
			return &itm;
	}

	return nullptr;
}

/**
 * @brief convert IIQ/IIR to normalized value
 * @param player player's stat value (200% increased = 200)
 * @param map map's stat value (200% increased = 200)
 * @return value in normalized format (1.0 meaning no difference)
 * @details example:
 * for player's +200% (3x more) and map's +200% (3x more) the result is 9.0 (9x normal stat)
 */
[[nodiscard]] double increased_item_stats_normalized(int player, int map)
{
	return ((player + 100) / 100.0) * ((map + 100) / 100.0);
}

} // namespace

namespace fs::gui {

bool loot_button_plurality::draw(const char* str)
{
	aux::scoped_pointer_id _(this);
	ImGui::DragInt4("", _min_max, 0.25f, 0, INT_MAX);

	if (!ImGui::IsItemActive()) {
		if (min_quantity() > max_quantity())
			std::swap(min_quantity(), max_quantity());

		if (min_stack_size() > max_stack_size())
			std::swap(min_stack_size(), max_stack_size());
	}

	ImGui::SameLine();
	return ImGui::Button(str);
}

bool loot_button_drag_range::draw(const char* str)
{
	aux::scoped_pointer_id _(this);
	ImGui::DragInt2("", _min_max, 0.25f, 0, INT_MAX);

	if (!ImGui::IsItemActive() && min() > max())
		std::swap(min(), max());

	ImGui::SameLine();
	return ImGui::Button(str);
}

void loot_drag_range::draw(const char* str)
{
	aux::scoped_pointer_id _(this);
	ImGui::DragInt2(str, _min_max, 0.25f, 0, INT_MAX);

	if (!ImGui::IsItemActive() && min() > max())
		std::swap(min(), max());
}

void loot_slider_range::draw(const char* str)
{
	aux::scoped_pointer_id _(this);
	ImGui::SliderInt2(str, _min_max, _limit.min, _limit.max);

	if (!ImGui::IsItemActive() && min() > max())
		std::swap(min(), max());
}

void loot_state::draw_interface(application& app, filter_state_mediator& mediator)
{
	const std::optional<lang::loot::item_database>& database = app.item_database();
	if (!database) {
		ImGui::TextWrapped("Item database failed to load. Loot generation not available.");
		return;
	}

	if (ImGui::Button("Clear"))
		clear_items(mediator);

	ImGui::SameLine();
	ImGui::Checkbox("Show hidden items", &_show_hidden_items);
	ImGui::SameLine();
	ImGui::Checkbox("Append mode", &_append_loot);
	aux::on_hover_text_tooltip("By default, new items replace existing ones.\nEnable this to append new items instead.");
	if (_append_loot) {
		ImGui::SameLine();
		ImGui::Checkbox("Shuffle", &_shuffle_loot);
		aux::on_hover_text_tooltip("Shuffle items when new ones are generated.");
	}

	draw_loot_canvas(app.font_settings(), mediator);

	ImGui::BeginChild("loot buttons");

	const lang::loot::item_database& db = *database;
	draw_loot_settings_global();
	draw_loot_buttons_currency        (db, app.loot_generator());
	draw_loot_buttons_specific_classes(db, app.loot_generator());
	draw_loot_buttons_gems            (db, app.loot_generator());
	draw_loot_buttons_equipment       (db, app.loot_generator());

	ImGui::EndChild();

	if (_last_items_size != _items.size()) {
		if (_last_items_size < _items.size() && !_append_loot) {
			const auto first = _items.begin();
			const auto last  = _items.begin() + _last_items_size;

			for (auto it = first; it != last; ++it) {
				if (it->filtering_result && !(*it->filtering_result).style.visibility.show)
					--_num_hidden_items;
			}

			_items.erase(first, last);
			_canvas_offset_y = 0.0f;
		}

		if (_shuffle_loot)
			std::shuffle(_items.begin(), _items.end(), app.loot_generator().rng());

		_last_items_size = _items.size();
		mediator.on_loot_change();
	}
}

void loot_state::draw_loot_settings_global()
{
	if (!ImGui::CollapsingHeader("Global settings", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	ImGui::SliderInt("area level", &_area_level, 1, lang::limits::max_item_level);
	if (_area_level > 67) {
		ImGui::SameLine(0, 0);
		ImGui::Text(" (tier %d)", _area_level - 67);
	}

	constexpr auto mf_min = -100;
	constexpr auto mf_max = 400;
	constexpr auto mf_format = "%+d%%";
	ImGui::SliderInt("player IIR", &_player_iir, mf_min, mf_max, mf_format);
	ImGui::SliderInt("map IIR",    &_map_iir,    mf_min, mf_max, mf_format);
	draw_percent_slider("chance to identify", _chance_to_identify);
	draw_percent_slider("chance to corrupt",  _chance_to_corrupt);
	draw_percent_slider("chance to mirror",   _chance_to_mirror);
}

void loot_state::draw_loot_buttons_currency(const lang::loot::item_database& db, lang::loot::generator& gen)
{
	if (!ImGui::CollapsingHeader("Currency (min/max quantity, min/max stack size)", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	if (_currency_generic.draw("generic"))
		gen.generate_generic_currency(db, *this, _currency_generic.plurality(), _area_level);
	if (_currency_generic_shards.draw("shards"))
		gen.generate_generic_currency_shards(db, *this, _currency_generic_shards.plurality(), _area_level);
	if (_currency_conqueror_orbs.draw("conqueror"))
		gen.generate_conqueror_orbs(db, *this, _currency_conqueror_orbs.plurality(), _area_level);
	if (_currency_breach_blessings.draw("breach blessings"))
		gen.generate_breach_blessings(db, *this, _currency_breach_blessings.plurality(), _area_level);
	if (_currency_breach_splinters.draw("breach splinters"))
		gen.generate_breach_splinters(db, *this, _currency_breach_splinters.plurality(), _area_level);
	if (_currency_legion_splinters.draw("legion splinters"))
		gen.generate_legion_splinters(db, *this, _currency_legion_splinters.plurality(), _area_level);
	if (_currency_essences.draw("essences"))
		gen.generate_essences(db, *this,_currency_essences.plurality(), _area_level);
	if (_currency_fossils.draw("fossils"))
		gen.generate_fossils(db, *this, _currency_fossils.plurality(), _area_level);
	if (_currency_catalysts.draw("catalysts"))
		gen.generate_catalysts(db, *this, _currency_catalysts.plurality(), _area_level);
	if (_currency_oils.draw("blight oils"))
		gen.generate_oils(db, *this, _currency_oils.plurality(), _area_level);
	if (_currency_delirium_orbs.draw("delirium orbs"))
		gen.generate_delirium_orbs(db, *this, _currency_delirium_orbs.plurality(), _area_level);
	if (_currency_harbinger_scrolls.draw("harbinger scrolls"))
		gen.generate_harbinger_scrolls(db, *this, _currency_harbinger_scrolls.plurality(), _area_level);
	if (_currency_incursion_vials.draw("incursion vials"))
		gen.generate_incursion_vials(db, *this, _currency_incursion_vials.plurality(), _area_level);
	if (_currency_bestiary_nets.draw("bestiary nets"))
		gen.generate_bestiary_nets(db, *this, _currency_bestiary_nets.plurality(), _area_level);
}

void loot_state::draw_loot_buttons_specific_classes(const lang::loot::item_database& db, lang::loot::generator& gen)
{
	if (!ImGui::CollapsingHeader("Items of specific class", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	if (_class_cards.draw("divination cards"))
		gen.generate_divination_cards(db, *this, _class_cards.plurality());
	if (_class_resonators.draw("resonators"))
		gen.generate_resonators(db, *this, _class_resonators.plurality(), _area_level);
	if (_class_incubators.draw("incubators"))
		gen.generate_incubators(db, *this, _class_incubators.range(), _area_level);
	if (_class_metamorphs.draw("metamorph parts"))
		gen.generate_metamorph_parts(db, *this, _class_metamorphs.range(), _area_level);
	if (_class_unique_pieces.draw("unique pieces"))
		gen.generate_unique_pieces(db, *this, _class_unique_pieces.range(), _area_level, _chance_to_corrupt);
	if (_class_lab_keys.draw("lab keys"))
		gen.generate_labyrinth_keys(db, *this, _class_lab_keys.range());
	if (_class_lab_trinkets.draw("lab trinkets"))
		gen.generate_labyrinth_trinkets(db, *this, _class_lab_trinkets.range());
	if (_class_quest_items.draw("quest items"))
		gen.generate_quest_items(db, *this, _class_quest_items.range());
}

void loot_state::draw_loot_buttons_gems(const lang::loot::item_database& db, lang::loot::generator& gen)
{
	if (!ImGui::CollapsingHeader("Gems", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	if (ImGui::Button("generate")) {
		gen.generate_gems(db, *this, _gems_quantity.range(), _gems_level.range(), _gems_quality.range(),
			_area_level, _chance_to_corrupt, _gems_gem_types);
	}

	ImGui::SameLine();
	ImGui::Checkbox("Active", &_gems_gem_types.active);
	ImGui::SameLine();
	ImGui::Checkbox("Vaal active", &_gems_gem_types.vaal_active);
	ImGui::SameLine();
	ImGui::Checkbox("Support", &_gems_gem_types.support);
	ImGui::SameLine();
	ImGui::Checkbox("Awakened support", &_gems_gem_types.awakened_suport);
	_gems_quantity.draw("quantity");
	_gems_level.draw("level");
	_gems_quality.draw("quality");
}

void loot_state::draw_loot_buttons_equipment(const lang::loot::item_database& db, lang::loot::generator& gen)
{
	if (!ImGui::CollapsingHeader("Equipment (aka rare garbage)", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	constexpr auto class_weights_popup_str = "class weights";
	if (ImGui::Button(class_weights_popup_str))
		ImGui::OpenPopup(class_weights_popup_str);

	if (ImGui::BeginPopup(class_weights_popup_str)) {
		ImGui::TextUnformatted("main parts");
		draw_weight_drag("body armours", _eq_class_weights.body_armours);
		draw_weight_drag("helmets", _eq_class_weights.helmets);
		draw_weight_drag("gloves", _eq_class_weights.gloves);
		draw_weight_drag("boots", _eq_class_weights.boots);
		ImGui::TextUnformatted("jewellery");
		draw_weight_drag("amulets", _eq_class_weights.amulets);
		draw_weight_drag("rings", _eq_class_weights.rings);
		draw_weight_drag("belts", _eq_class_weights.belts);
		ImGui::TextUnformatted("1-handed");
		draw_weight_drag("axes", _eq_class_weights.axes_1h);
		draw_weight_drag("maces", _eq_class_weights.maces_1h);
		draw_weight_drag("swords", _eq_class_weights.swords_1h);
		draw_weight_drag("thrusting swords", _eq_class_weights.thrusting_swords);
		draw_weight_drag("claws", _eq_class_weights.claws);
		draw_weight_drag("daggers", _eq_class_weights.daggers);
		draw_weight_drag("rune daggers", _eq_class_weights.rune_daggers);
		draw_weight_drag("wands", _eq_class_weights.wands);
		ImGui::TextUnformatted("2-handed");
		draw_weight_drag("axes", _eq_class_weights.axes_2h);
		draw_weight_drag("maces", _eq_class_weights.maces_2h);
		draw_weight_drag("swords", _eq_class_weights.swords_2h);
		draw_weight_drag("staves", _eq_class_weights.staves);
		draw_weight_drag("warstaves", _eq_class_weights.warstaves);
		draw_weight_drag("bows", _eq_class_weights.bows);
		draw_weight_drag("fishing rods", _eq_class_weights.fishing_rods);
		ImGui::TextUnformatted("offhand");
		draw_weight_drag("shields", _eq_class_weights.shields);
		draw_weight_drag("quivers", _eq_class_weights.quivers);
		ImGui::EndPopup();
	}

	ImGui::SameLine();

	constexpr auto influence_weights_popup_str = "influence weights";
	if (ImGui::Button(influence_weights_popup_str))
		ImGui::OpenPopup(influence_weights_popup_str);

	if (ImGui::BeginPopup(influence_weights_popup_str)) {
		draw_weight_drag("None",     _eq_influence_weights.none);
		draw_weight_drag("Shaper",   _eq_influence_weights.shaper);
		draw_weight_drag("Elder",    _eq_influence_weights.elder);
		draw_weight_drag("Crusader", _eq_influence_weights.crusader);
		draw_weight_drag("Redeemer", _eq_influence_weights.redeemer);
		draw_weight_drag("Hunter",   _eq_influence_weights.hunter);
		draw_weight_drag("Warlord",  _eq_influence_weights.warlord);
		ImGui::EndPopup();
	}

	_eq_quality.draw("quality");

	/*
	 * This manual sizing is ugly but Dear ImGui has no support for "allocating just enough space"
	 * for child windows. Child window here is needed because columns are global-per-window
	 * which artificially limits library users to create subcolumns due to single global state.
	 * Everything through global functions and singletons. Good design, isn't it?
	 */
	const auto& style = ImGui::GetStyle();
	const auto child_height = 10.0f * (style.ItemSpacing.y + style.FramePadding.y + ImGui::GetFontSize());
	if (ImGui::BeginChild("equipment generation", ImVec2(0.0f, child_height))) {
		ImGui::Columns(2);

		if (ImGui::Button("generate - fixed weights")) {
			gen.generate_equippable_items_fixed_weights(
				db, *this, _eq_quality.range(), _eq_fixed_weights_quantity.range(),
				_area_level, _chance_to_identify, _chance_to_corrupt, _chance_to_mirror,
				_eq_class_weights, _eq_influence_weights,
				_eq_fixed_weights_rarity, _eq_fixed_weights_item_level);
		}

		// fixed weights generation
		_eq_fixed_weights_quantity.draw("quantity");
		draw_weight_drag("Rarity: Normal", _eq_fixed_weights_rarity.normal);
		draw_weight_drag("Rarity: Magic",  _eq_fixed_weights_rarity.magic);
		draw_weight_drag("Rarity: Rare ",  _eq_fixed_weights_rarity.rare);
		draw_weight_drag("ilvl = area", _eq_fixed_weights_item_level.base);
		draw_weight_drag("ilvl +1",     _eq_fixed_weights_item_level.plus_one);
		draw_weight_drag("ilvl +2",     _eq_fixed_weights_item_level.plus_two);

		ImGui::NextColumn();

		if (ImGui::Button("generate - simulated monster pack")) {
			gen.generate_equippable_items_monster_pack(
				db, *this, _eq_quality.range(),
				_eq_monster_pack_normal.range(), _eq_monster_pack_magic.range(), _eq_monster_pack_rare.range(), _eq_monster_pack_unique.range(),
				_area_level, increased_item_stats_normalized(_player_iir, _map_iir),
				_chance_to_identify, _chance_to_corrupt, _chance_to_mirror,
				_eq_class_weights, _eq_influence_weights);
		}

		// monster pack generation
		_eq_monster_pack_normal.draw("normal monsters");
		_eq_monster_pack_magic.draw("magic monsters");
		_eq_monster_pack_rare.draw("rare monsters");
		_eq_monster_pack_unique.draw("unique monsters");
		ImGui::Columns(1);
	}
	ImGui::EndChild();
}

void loot_state::draw_loot_canvas(const fonting& f, filter_state_mediator& mediator)
{
	const ImVec2 canvas_begin = ImGui::GetCursorScreenPos(); // in screen coordinates
	const ImVec2 canvas_size(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 8.0f);
	const ImVec2 canvas_end(canvas_begin.x + canvas_size.x, canvas_begin.y + canvas_size.y);

	ImDrawList* const draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(canvas_begin, canvas_end, IM_COL32(50, 50, 50, 255)); // draw background
	draw_list->AddRect(canvas_begin, canvas_end, IM_COL32(255, 255, 255, 255)); // draw border

	draw_item_labels(canvas_begin, canvas_end, f);

	/*
	 * The line below has multiple purposes:
	 * - Tells Dear ImGui to catch user interactions which allows to use IsItemHovered(), IsItemActive() etc
	 * - Advances Dear ImGui's drawing position for further elements.
	 *
	 * Note: IsItem* functions below apply to the last drawn item, if there is any further code that draws more items
	 * then IsItem* functions should be called immediately after InvisibleButton with results saved to local variables.
	 */
	ImGui::InvisibleButton("canvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

	const auto& io = ImGui::GetIO();
	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		// right now only vertical scrolling
		_canvas_offset_y += io.MouseDelta.y;
	}

	const bool is_canvas_hovered = ImGui::IsItemHovered();

	if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
		on_canvas_right_click(io.MousePos, mediator);
	}

	if (is_canvas_hovered) {
		_canvas_offset_y += io.MouseWheel * ImGui::GetFontSize();

		if (!_items.empty())
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll); // this lasts only 1 frame

		on_canvas_hover(io.MousePos, f);
	}

	// block from top because items are being rendered top-to-bottom
	// there is no reason to scroll upwards of top elements
	if (_canvas_offset_y > 0.0f)
		_canvas_offset_y = 0.0f;
}

void loot_state::draw_item_labels(ImVec2 canvas_begin, ImVec2 canvas_end, const fonting& f)
{
	ImDrawList* const draw_list = ImGui::GetWindowDrawList();
	draw_list->PushClipRect(canvas_begin, canvas_end, true);

	ImVec2 current_position = canvas_begin;
	float row_height = 0.0f;
	for (looted_item& itm : _items) {
		if (itm.filtering_result) {
			const auto& style = *itm.filtering_result;

			if (!_show_hidden_items && !style.style.visibility.show) {
				itm.drawing = std::nullopt;
				continue;
			}

			const item_label_data ild(itm.itm, style.style.font_size.size.value, f);
			const ImVec2 item_size = ild.whole_size();

			// If the item does not fit in the current row...
			if (current_position.x + item_size.x > canvas_end.x) {
				// ...move to the next row.
				// In case the item drawn is first in the row this code is no-op and the item is drawn anyway.
				// This is desired because in case even a single item does not fit in a row then the best
				// possible thing to do is to render it clipped.
				current_position.x = canvas_begin.x;
				current_position.y += row_height;
				row_height = 0.0f;
			}

			const ImVec2 item_begin(current_position.x /* + _canvas_offset_x */, current_position.y + _canvas_offset_y);

			if (item_begin.y > canvas_end.y)
				break; // stop the loop - no more items in visible canvas area

			draw_item_label(item_begin, ild, itm.itm, *itm.filtering_result, f);

			itm.drawing = aux::rect{item_begin, item_size};

			row_height = std::max(row_height, item_size.y);
			current_position.x += item_size.x;
		}
	}

	draw_list->PopClipRect();
}

void loot_state::on_canvas_hover(ImVec2 mouse_position, const fonting& f)
{
	const looted_item* const ptr = find_item_by_mouse_position(_items, mouse_position);
	if (ptr == nullptr)
		return;

	const auto& itm = *ptr;
	if (itm.filtering_result)
		draw_item_tooltip(itm.itm, *itm.filtering_result, f);
}

void loot_state::on_canvas_right_click(ImVec2 mouse_position, filter_state_mediator& mediator)
{
	const looted_item* const ptr = find_item_by_mouse_position(_items, mouse_position);
	if (ptr == nullptr)
		return;

	FS_ASSERT_MSG(
		ptr->filtering_result.has_value(),
		"Items found by mouse position must have been rendered and therefore filtered");

	mediator.on_debug_open(ptr->itm, *ptr->filtering_result);
}

void loot_state::clear_items(filter_state_mediator& mediator)
{
	_items.clear();
	_last_items_size = 0;
	_num_hidden_items = 0;
	_canvas_offset_y = 0.0f;
	mediator.on_loot_change();
}

void loot_state::clear_filter_results(filter_state_mediator& mediator)
{
	for (looted_item& itm : _items)
		itm.filtering_result = std::nullopt;

	mediator.on_filter_results_clear();
}

void loot_state::update_items(const lang::item_filter& filter)
{
	for (looted_item& itm : _items) {
		if (!itm.filtering_result) {
			itm.filtering_result = lang::pass_item_through_filter(itm.itm, filter, _area_level);

			if (!(*itm.filtering_result).style.visibility.show)
				++_num_hidden_items;
		}
	}
}

void loot_state::refilter_items(const lang::item_filter& filter, filter_state_mediator& mediator)
{
	_num_hidden_items = 0;

	for (looted_item& itm : _items) {
		itm.filtering_result = lang::pass_item_through_filter(itm.itm, filter, _area_level);

		if (!(*itm.filtering_result).style.visibility.show)
			++_num_hidden_items;
	}

	mediator.on_filter_results_change();
}

}
