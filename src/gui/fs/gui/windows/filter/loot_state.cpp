#include <fs/gui/windows/filter/loot_state.hpp>
#include <fs/gui/application.hpp>
#include <fs/gui/settings/fonting.hpp>
#include <fs/gui/ui_utils.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/lang/limits.hpp>

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cstdio>

namespace {

using namespace fs;

class item_label_data
{
public:
	item_label_data(const lang::item& itm, int font_size, const gui::fonting& f)
	{
		if (itm.name)
			_first_line_size = measure_text_line(*itm.name, font_size, f.filter_preview_font(font_size));

		if (itm.stack_size == 1)
			std::snprintf(_second_line_buf.data(), _second_line_buf.size(), "%s", itm.base_type.c_str());
		else
			std::snprintf(_second_line_buf.data(), _second_line_buf.size(), "%dx %s", itm.stack_size, itm.base_type.c_str());

		_second_line_size = measure_text_line(_second_line_buf.data(), font_size, f.filter_preview_font(font_size));

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
	static constexpr auto padding_x = 10.0f;
	static constexpr auto padding_y = 5.0f;

private:
	static ImVec2 measure_text_line(std::string_view line, int font_size, const ImFont* fnt)
	{
		return fnt->CalcTextSizeA(font_size, FLT_MAX, 0.0f, line.data(), line.data() + line.size());
	}

	std::array<char, 40> _second_line_buf;
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

void draw_item_tooltip(const lang::item& itm, const lang::item_filtering_result& result, const gui::fonting& f)
{
	const auto _1 = f.scoped_monospaced_font();
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 30.0f, 0));
	gui::scoped_tooltip _2;

	ImGui::Columns(2);

	{
		namespace kw = lang::keywords::rf;

		ImGui::TextDisabled("name:");
		ImGui::Text("%s:", kw::base_type);
		ImGui::Text("%s:", kw::class_);
		ImGui::Text("%s:", kw::rarity);
		ImGui::Text("%s:", kw::sockets);
		ImGui::Text("%s:", kw::item_level);
		ImGui::Text("%s:", kw::drop_level);
		ImGui::Text("%s:", kw::width);
		ImGui::Text("%s:", kw::height);
		ImGui::Text("%s:", kw::quality);
		ImGui::Text("%s:", kw::stack_size);
		ImGui::Text("%s:", kw::gem_level);
		ImGui::Text("%s:", kw::map_tier);
		ImGui::Text("%s:", kw::corrupted_mods);

		ImGui::Text("%s:", kw::has_influence);
		ImGui::Text("%s:", kw::has_explicit_mod);
		ImGui::Text("%s:", kw::has_enchantment);
		ImGui::Text("%s:", kw::enchantment_passive_node);
		ImGui::TextDisabled("annointments:");

		ImGui::Text("%s:", kw::prophecy);
		ImGui::Text("%s:", kw::identified);
		ImGui::Text("%s:", kw::corrupted);
		ImGui::Text("%s:", kw::mirrored);
		ImGui::Text("%s:", kw::fractured_item);
		ImGui::Text("%s:", kw::synthesised_item);
		ImGui::Text("%s:", kw::shaped_map);
		ImGui::Text("%s:", kw::elder_map);
		ImGui::Text("%s:", kw::blighted_map);
		ImGui::Text("%s:", kw::replica);
		ImGui::Text("%s:", kw::alternate_quality);
	}

	ImGui::NextColumn();

	{
		if (itm.name)
			ImGui::TextUnformatted((*itm.name).c_str());
		else
			ImGui::TextUnformatted("-");

		ImGui::TextUnformatted(itm.base_type.c_str());
		ImGui::TextUnformatted(itm.class_.c_str());

		const auto rarity = to_string_view(itm.rarity_);
		ImGui::TextUnformatted(rarity.data(), rarity.data() + rarity.size()); // TODO add colors

		ImGui::TextUnformatted(to_string(itm.sockets).c_str()); // TODO add colors
		ImGui::Text("%d", itm.item_level);
		ImGui::Text("%d", itm.drop_level);
		ImGui::Text("%d", itm.width);
		ImGui::Text("%d", itm.height);
		ImGui::Text("%d", itm.quality);
		ImGui::Text("%d", itm.stack_size);
		ImGui::Text("%d", itm.gem_level);
		ImGui::Text("%d", itm.map_tier);
		ImGui::Text("%d", itm.corrupted_mods);

		if (itm.influence.is_none()) {
			ImGui::TextUnformatted("None");
		}
		else {
			bool first_influence_text = true;
			const auto output_influence = [&](bool condition, const char* name) {
				if (!condition)
					return;

				if (!first_influence_text) {
					ImGui::SameLine(0, 0);
				}

				ImGui::Text("%s ", name);
				first_influence_text = false;
			};

			// TODO colors?
			namespace kw = lang::keywords::rf;
			output_influence(itm.influence.shaper, kw::shaper);
			output_influence(itm.influence.elder, kw::elder);
			output_influence(itm.influence.crusader, kw::crusader);
			output_influence(itm.influence.redeemer, kw::redeemer);
			output_influence(itm.influence.hunter, kw::hunter);
			output_influence(itm.influence.warlord, kw::warlord);
		}

		ImGui::TextUnformatted("TODO");
		ImGui::TextUnformatted("TODO");
		ImGui::TextUnformatted("TODO");
		ImGui::TextUnformatted("TODO");

		if (itm.is_prophecy) {
			// string here because filters expect this string
			// for unknown reason, Prophecy is not a boolean condition
			ImGui::TextUnformatted(itm.base_type.c_str());
		}
		else {
			ImGui::TextUnformatted("-");
		}

		const auto output_boolean = [](bool value) {
			namespace kw = lang::keywords::rf;

			if (value)
				ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(gui::color_true), kw::true_);
			else
				ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(gui::color_false), kw::false_);
		};

		output_boolean(itm.is_identified);
		output_boolean(itm.is_corrupted);
		output_boolean(itm.is_mirrored);
		output_boolean(itm.is_fractured_item);
		output_boolean(itm.is_synthesised_item);
		output_boolean(itm.is_shaped_map);
		output_boolean(itm.is_elder_map);
		output_boolean(itm.is_blighted_map);
		output_boolean(itm.is_replica);
		output_boolean(itm.is_alternate_quality);
	}

	ImGui::Columns(1); // reset back to single column

	ImGui::Dummy({0.0f, ImGui::GetFontSize() * 1.0f});
	ImGui::TextUnformatted("filter conditions");
	ImGui::TextDisabled("properties unsupported by filters");
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

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
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
}

} // namespace

namespace fs::gui {

bool loot_button_with_drags::draw(const char* str)
{
	scoped_pointer_id _(this);
	const bool result = ImGui::Button(str);
	ImGui::SameLine();
	ImGui::DragInt4("", _min_max, 0.25f, 0, INT_MAX);

	if (!ImGui::IsItemActive()) {
		if (min_quantity() > max_quantity())
			std::swap(min_quantity(), max_quantity());

		if (min_stack_size() > max_stack_size())
			std::swap(min_stack_size(), max_stack_size());
	}

	return result;
}

void loot_state::draw_interface(application& app)
{
	const std::optional<lang::loot::item_database>& database = app.item_database();
	if (!database) {
		ImGui::TextWrapped("Item database failed to load. Loot generation not available.");
		return;
	}

	if (ImGui::Button("Clear"))
		clear_items();

	ImGui::SameLine();
	ImGui::TextWrapped("Hover items for info, right click items for debug, drag or scroll to move");

	draw_loot_canvas(app.font_settings());

	const lang::loot::item_database& db = *database;

	draw_loot_settings_global();
	draw_loot_buttons_currency(db, app.loot_generator());

	if (_last_items_size != _items.size()) {
		if (_last_items_size < _items.size() && !_append_loot) {
			_items.erase(_items.begin(), _items.begin() + _last_items_size);
			_canvas_offset_y = 0.0f;
		}

		if (_shuffle_loot)
			std::shuffle(_items.begin(), _items.end(), app.loot_generator().rng());

		_last_items_size = _items.size();
	}
}

void loot_state::draw_loot_settings_global()
{
	if (!ImGui::CollapsingHeader("Global settings", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	ImGui::Checkbox("Append new items instead of replacing", &_append_loot);
	if (_append_loot)
		ImGui::Checkbox("Shuffle items when new ones are generated", &_shuffle_loot);

	ImGui::SliderInt("Area level", &_area_level, 1, lang::limits::max_item_level);

	constexpr auto mf_min = -100;
	constexpr auto mf_max = 400;
	constexpr auto mf_format = "%+d%%";
	ImGui::SliderInt("Player IIQ", &_player_iiq, mf_min, mf_max, mf_format);
	ImGui::SliderInt("Player IIR", &_player_iir, mf_min, mf_max, mf_format);
	ImGui::SliderInt("Map IIQ",    &_map_iiq,    mf_min, mf_max, mf_format);
	ImGui::SliderInt("Map IIR",    &_map_iir,    mf_min, mf_max, mf_format);
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

void loot_state::draw_loot_canvas(const fonting& f)
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

	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		const auto& io = ImGui::GetIO();
		// right now only vertical scrolling
		_canvas_offset_y += io.MouseDelta.y;
	}

	if (ImGui::IsItemHovered()) {
		const auto& io = ImGui::GetIO();
		_canvas_offset_y += io.MouseWheel * ImGui::GetFontSize();
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

			itm.drawing = rect{item_begin, item_size};

			row_height = std::max(row_height, item_size.y);
			current_position.x += item_size.x;
		}
	}

	draw_list->PopClipRect();
}

void loot_state::on_canvas_hover(ImVec2 mouse_position, const fonting& f)
{
	for (looted_item& itm : _items) {
		if (itm.drawing) {
			if ((*itm.drawing).contains(mouse_position)) {
				FS_ASSERT(itm.filtering_result.has_value());
				draw_item_tooltip(itm.itm, *itm.filtering_result, f);
				break;
			}
		}
	}
}

void loot_state::update_items(const lang::item_filter& filter)
{
	for (looted_item& itm : _items)
		if (!itm.filtering_result)
			itm.filtering_result = lang::pass_item_through_filter(itm.itm, filter, _area_level);
}

void loot_state::refilter_items(const lang::item_filter& filter)
{
	for (looted_item& itm : _items)
		itm.filtering_result = lang::pass_item_through_filter(itm.itm, filter, _area_level);
}

}
