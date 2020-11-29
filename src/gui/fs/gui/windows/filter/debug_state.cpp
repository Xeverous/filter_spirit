#include <fs/gui/windows/filter/debug_state.hpp>
#include <fs/gui/windows/filter/item_tooltip.hpp>
#include <fs/gui/settings/fonting.hpp>
#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/parser/parser.hpp>
#include <fs/lang/item.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/utility/assert.hpp>

#include <imgui.h>

#include <string_view>
#include <string>

namespace {

using namespace fs;

constexpr ImU32 color_text                          = IM_COL32(255, 255, 255, 255);
constexpr ImU32 color_line_number                   = IM_COL32(200, 200, 200, 255);
constexpr ImU32 color_background_default            = IM_COL32(  0,   0,   0, 255);
constexpr ImU32 color_background_condition_success  = IM_COL32(  0, 127,   0, 255);
constexpr ImU32 color_background_condition_failure  = IM_COL32(127,   0,   0, 255);
constexpr ImU32 color_background_matched_continue   = IM_COL32(  0, 127, 255, 255);
constexpr ImU32 color_background_matched_visibility = IM_COL32(  0, 127, 255, 255);
constexpr ImU32 color_background_matched_action     = IM_COL32(127,  63, 192, 255);

std::size_t origins_line_number(
	const parser::lookup_data& lookup,
	const parser::line_lookup& lines,
	const lang::position_tag origin)
{
	const std::string_view sv = lookup.position_of(origin);
	const parser::text_range r = lines.text_range_for(sv);
	return r.first.line_number;
}

void color_line_by_condition_result(
	const std::optional<lang::condition_match_result>& opt_result,
	const parser::lookup_data& lookup,
	const parser::line_lookup& lines,
	std::vector<ImU32>& line_colors)
{
	if (!opt_result)
		return;

	const lang::condition_match_result& result = *opt_result;
	const lang::position_tag origin = result.condition_origin();

	if (!lang::is_valid(origin))
		return;

	const std::size_t line_number = origins_line_number(lookup, lines, origin);
	FS_ASSERT(line_number < line_colors.size());

	if (result.is_successful())
		line_colors[line_number] = color_background_condition_success;
	else
		line_colors[line_number] = color_background_condition_failure;
}

void color_line_by_condition_result(
	const lang::range_condition_match_result& result,
	const parser::lookup_data& lookup,
	const parser::line_lookup& lines,
	std::vector<ImU32>& line_colors)
{
	color_line_by_condition_result(result.lower_bound, lookup, lines, line_colors);
	color_line_by_condition_result(result.upper_bound, lookup, lines, line_colors);
}

template <typename T>
void color_line_by_origin(
	const T& t,
	const parser::lookup_data& lookup,
	const parser::line_lookup& lines,
	std::vector<ImU32>& line_colors,
	ImU32 color)
{
	const lang::position_tag origin = t.origin;

	if (!lang::is_valid(origin))
		return;

	const std::size_t line_number = origins_line_number(lookup, lines, origin);
	FS_ASSERT(line_number < line_colors.size());
	line_colors[line_number] = color;
}

template <typename Action>
void color_line_by_action_origin(
	const Action& action,
	const parser::lookup_data& lookup,
	const parser::line_lookup& lines,
	std::vector<ImU32>& line_colors)
{
	color_line_by_origin(action, lookup, lines, line_colors, color_background_matched_action);
}

template <typename Action>
void color_line_by_action_origin(
	const std::optional<Action>& opt_action,
	const parser::lookup_data& lookup,
	const parser::line_lookup& lines,
	std::vector<ImU32>& line_colors)
{
	if (!opt_action)
		return;

	color_line_by_action_origin(*opt_action, lookup, lines, line_colors);
}

float calculate_line_number_column_width(std::size_t num_lines)
{
	return gui::aux::measure_text_line(
		std::to_string(num_lines),
		ImGui::GetFontSize(),
		ImGui::GetFont()).x;
}

void draw_line_numbers(ImVec2 begin, ImVec2 end, float offset_y, std::size_t num_lines)
{
	ImDrawList* const draw_list = ImGui::GetWindowDrawList();
	const float line_height = ImGui::GetTextLineHeight();
	const int min_str_width = std::to_string(num_lines).length();

	ImVec2 pos(begin.x, begin.y + offset_y);
	std::array<char, 10> buf;

	for (std::size_t i = 0; i < num_lines; ++i) {
		std::snprintf(buf.data(), buf.size(), "%*zu", min_str_width, i + 1);
		draw_list->AddText(pos, color_line_number, buf.data());
		pos.y += line_height;

		if (pos.y >= end.y)
			break;
	}
}

void draw_text(
	ImVec2 begin,
	ImVec2 end,
	ImVec2 offset,
	const parser::line_lookup& lines,
	const std::vector<ImU32>& line_colors)
{
	const float line_width = end.x - begin.x;
	const float line_height = ImGui::GetTextLineHeight();
	ImVec2 pos(begin.x + offset.x, begin.y + offset.y);
	ImDrawList* const draw_list = ImGui::GetWindowDrawList();

	for (std::size_t i = 0; i < lines.num_lines(); ++i) {
		draw_list->AddRectFilled(pos, {pos.x + line_width, pos.y + line_height}, line_colors[i]);
		const std::string_view line = lines.get_line(i);

		if (!line.empty()) // Dear ImGui does not work with empty ranges
			draw_list->AddText(pos, color_text, line.data(), line.data() + line.size());

		pos.y += line_height;

		if (pos.y >= end.y)
			break;
	}
}

float draw_colored_source(
	const parser::line_lookup& lines,
	const std::vector<ImU32>& line_colors,
	float offset_y)
{
	FS_ASSERT(lines.num_lines() == line_colors.size());

	if (ImGui::BeginChild("source")) {
		const ImVec2 begin = ImGui::GetCursorScreenPos();
		const ImVec2 free_space = ImGui::GetContentRegionAvail();
		const ImVec2 end(begin.x + free_space.x, begin.y + free_space.y);

		const float line_number_column_width =
			calculate_line_number_column_width(lines.num_lines()) + ImGui::GetFontSize() / 2.0f;
		draw_line_numbers(begin, end, offset_y, lines.num_lines());
		draw_text({begin.x + line_number_column_width, begin.y}, end, {0.0f, offset_y}, lines, line_colors);

		ImGui::InvisibleButton("debug", free_space);

		if (ImGui::IsItemHovered()) {
			offset_y += ImGui::GetIO().MouseWheel * ImGui::GetTextLineHeight() * 5.0f;

			if (offset_y > 0.0f)
				offset_y = 0.0f;
		}
	}
	ImGui::EndChild();

	return offset_y;
}

float draw_debug_interface_impl(
	const lang::item& itm,
	const lang::item_filtering_result& /* result */,
	const parser::line_lookup& lines,
	const std::vector<ImU32>& line_colors,
	const gui::fonting& f,
	float offset_y)
{
	const auto _1 = f.scoped_monospaced_font();

	const auto first_column_width = get_item_tooltip_first_column_width(f);
	const auto second_column_width = get_item_tooltip_second_column_width(itm, f);

	ImGui::Columns(3);

	ImGui::SetColumnWidth(-1, first_column_width);
	gui::draw_item_tooltip_first_column();

	ImGui::NextColumn();

	ImGui::SetColumnWidth(-1, second_column_width);
	gui::draw_item_tooltip_second_column(itm);

	ImGui::NextColumn();

	float offset = draw_colored_source(lines, line_colors, offset_y);

	ImGui::Columns(1); // reset back to single column

	return offset;
}

constexpr auto debug_popup_title = "Item debug";

}

namespace fs::gui {

void debug_state::recompute()
{
	if (!_info)
		return;

	const debug_info& info = *_info;
	const parser::lookup_data& lookup = info.lookup;
	const parser::line_lookup& lines = info.lines;

	_line_colors.clear();
	_line_colors.resize(lines.num_lines(), color_background_default);

	// uncomment if item_filter is actually used within debug_state class
	// FS_ASSERT(info.result.get().match_history.size() <= info.filter.get().blocks.size());

	const lang::item_filtering_result& result = info.result.get();
	for (const lang::block_match_result& block : result.match_history) {
		/*
		 * Right not every condition is colored in the same way.
		 * Might want to do more precise coloring later
		 * (then specific condition results will have specific types).
		*/
		const lang::condition_set_match_result& cs = block.condition_set_result;
		color_line_by_condition_result(cs.item_level,               lookup, lines, _line_colors);
		color_line_by_condition_result(cs.drop_level,               lookup, lines, _line_colors);
		color_line_by_condition_result(cs.quality,                  lookup, lines, _line_colors);
		color_line_by_condition_result(cs.rarity,                   lookup, lines, _line_colors);
		color_line_by_condition_result(cs.linked_sockets,           lookup, lines, _line_colors);
		color_line_by_condition_result(cs.height,                   lookup, lines, _line_colors);
		color_line_by_condition_result(cs.width,                    lookup, lines, _line_colors);
		color_line_by_condition_result(cs.stack_size,               lookup, lines, _line_colors);
		color_line_by_condition_result(cs.gem_level,                lookup, lines, _line_colors);
		color_line_by_condition_result(cs.map_tier,                 lookup, lines, _line_colors);
		color_line_by_condition_result(cs.area_level,               lookup, lines, _line_colors);
		color_line_by_condition_result(cs.corrupted_mods,           lookup, lines, _line_colors);
		color_line_by_condition_result(cs.class_,                   lookup, lines, _line_colors);
		color_line_by_condition_result(cs.base_type,                lookup, lines, _line_colors);
		color_line_by_condition_result(cs.sockets,                  lookup, lines, _line_colors);
		color_line_by_condition_result(cs.socket_group,             lookup, lines, _line_colors);
		color_line_by_condition_result(cs.has_explicit_mod,         lookup, lines, _line_colors);
		color_line_by_condition_result(cs.has_enchantment,          lookup, lines, _line_colors);
		color_line_by_condition_result(cs.prophecy,                 lookup, lines, _line_colors);
		color_line_by_condition_result(cs.enchantment_passive_node, lookup, lines, _line_colors);
		color_line_by_condition_result(cs.has_influence,            lookup, lines, _line_colors);
		color_line_by_condition_result(cs.gem_quality_type,         lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_identified,            lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_corrupted,             lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_mirrored,              lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_elder_item,            lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_shaper_item,           lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_fractured_item,        lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_synthesised_item,      lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_enchanted,             lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_shaped_map,            lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_elder_map,             lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_blighted_map,          lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_replica,               lookup, lines, _line_colors);
		color_line_by_condition_result(cs.is_alternate_quality,     lookup, lines, _line_colors);

		if (block.continue_origin) {
			const std::size_t line_number = origins_line_number(lookup, lines, *block.continue_origin);
			FS_ASSERT(line_number < _line_colors.size());
			_line_colors[line_number] = color_background_matched_continue;
		}
	}

	const lang::item_style& style = result.style;
	color_line_by_action_origin(style.border_color,       lookup, lines, _line_colors);
	color_line_by_action_origin(style.text_color,         lookup, lines, _line_colors);
	color_line_by_action_origin(style.background_color,   lookup, lines, _line_colors);
	color_line_by_action_origin(style.font_size,          lookup, lines, _line_colors);
	color_line_by_action_origin(style.play_alert_sound,   lookup, lines, _line_colors);
	color_line_by_action_origin(style.switch_drop_sound,  lookup, lines, _line_colors);
	color_line_by_action_origin(style.minimap_icon,       lookup, lines, _line_colors);
	color_line_by_action_origin(style.play_effect,        lookup, lines, _line_colors);

	color_line_by_origin(style.visibility, lookup, lines, _line_colors, color_background_matched_visibility);
}

void debug_state::draw_interface(const fonting& f)
{
	if (_popup_pending) {
		ImGui::OpenPopup(debug_popup_title);
		_popup_pending = false;
		_popup_open = true;
	}

	if (!_info)
		return;

	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize); // take whole space
	if (ImGui::BeginPopupModal(debug_popup_title, &_popup_open, ImGuiWindowFlags_NoResize)) {
		const debug_info& info = *_info;
		_drawing_offset_y = draw_debug_interface_impl(info.itm.get(), info.result.get(), info.lines.get(), _line_colors, f, _drawing_offset_y);
		ImGui::EndPopup();
	}
}

}
