#include <fs/gui/windows/filter/loot_state.hpp>
#include <fs/gui/application.hpp>
#include <fs/gui/settings/fonting.hpp>
#include <fs/gui/ui_utils.hpp>

#include <imgui.h>

#include <algorithm>

namespace {

using namespace fs;

class item_preview_measurement
{
public:
	item_preview_measurement(const lang::item& itm, int font_size, const gui::fonting& f)
	{
		if (itm.name)
			_first_line_size = measure_text_line(*itm.name, font_size, f.filter_preview_font(font_size));

		_second_line_size = measure_text_line(itm.base_type, font_size, f.filter_preview_font(font_size));
	}

	ImVec2 whole_size() const
	{
		return {
			std::max(_first_line_size.x, _second_line_size.x) + 2.0f * padding_x,
			has_first_line() ?
				_first_line_size.y + _second_line_size.y + 3.0f * padding_y :
				_second_line_size.y + 2.0f * padding_y
		};
	}

	bool has_first_line() const
	{
		return _first_line_size.x != 0.0f && _first_line_size.y != 0.0f;
	}

	// use floats instead of Dear ImGui types because the library has no constexpr support
	static constexpr auto padding_x = 10.0f;
	static constexpr auto padding_y = 5.0f;

private:
	static ImVec2 measure_text_line(std::string_view line, int font_size, const ImFont* fnt)
	{
		return fnt->CalcTextSizeA(font_size, FLT_MAX, 0.0f, line.data(), line.data() + line.size());
	}

	ImVec2 _first_line_size;
	ImVec2 _second_line_size;
};

ImU32 to_imgui_color(lang::color c)
{
	if (c.a)
		return IM_COL32(c.r.value, c.g.value, c.b.value, (*c.a).value);
	else
		return IM_COL32(c.r.value, c.g.value, c.b.value, lang::limits::default_filter_opacity);
}

}

namespace fs::gui {

void loot_state::draw_interface(application& app)
{
	std::optional<lang::loot::item_database>& database = app.item_database();
	if (!database) {
		ImGui::TextWrapped("Item database failed to load. Loot generation not available.");
		return;
	}

	ImGui::TextWrapped("Hover items for info, right click items for debug, drag to move");

	draw_loot_canvas(app.font_settings());

	lang::loot::item_database& db = *database;

	if (ImGui::Button("random currency")) {
		clear_items();
		app.loot_generator().generate_generic_currency(db, *this, 10, lang::loot::stack_param::single);
	}
	if (ImGui::Button("single currency")) {
		clear_items();
		app.loot_generator().generate_generic_currency(db, *this, 1, lang::loot::stack_param::single);
	}
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
	 */
	ImGui::InvisibleButton("canvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

	if (ImGui::IsItemHovered()) {
		const auto& io = ImGui::GetIO();
		const ImVec2 origin(canvas_begin.x, canvas_begin.y + _canvas_offset_y);
		const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);
		on_canvas_hover(mouse_pos_in_canvas);
	}

	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		const auto& io = ImGui::GetIO();
		// right now only vertical scrolling
		_canvas_offset_y += io.MouseDelta.y;
	}
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

			item_preview_measurement ipm(itm.itm, style.style.font_size.size.value, f);
			const ImVec2 item_size = ipm.whole_size();

			if (current_position.x + item_size.x > canvas_end.x) {
				// move to the next row, if the item drawn is first in its raw this code is no-op
				current_position.x = canvas_begin.x;
				current_position.y += row_height;
				row_height = 0.0f;
			}

			const ImVec2 item_begin(current_position.x /* + _canvas_offset_x */, current_position.y + _canvas_offset_y);

			if (item_begin.y > canvas_end.y)
				break; // stop the loop - no more items in visible canvas area

			const ImVec2 item_end(item_begin.x + item_size.x, item_begin.y + item_size.y);
			draw_list->AddRectFilled(item_begin, item_end, to_imgui_color(style.style.background_color.c));
			if (style.style.border_color) {
				draw_list->AddRect(item_begin, item_end, to_imgui_color((*style.style.border_color).c));
			}

			itm.drawing = rect{item_begin, item_size};

			row_height = std::max(row_height, item_size.y);
			current_position.x += item_size.x;
		}
	}

	draw_list->PopClipRect();
}

void loot_state::on_canvas_hover(ImVec2 mouse_position)
{
	for (looted_item& itm : _items) {
		if (itm.drawing) {
			if ((*itm.drawing).contains(mouse_position)) {
				// TODO

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
