#include <fs/gui/windows/filter/debug_state.hpp>
#include <fs/gui/windows/filter/item_tooltip.hpp>
#include <fs/gui/settings/font_settings.hpp>
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
	const parser::parse_metadata& metadata,
	const lang::position_tag origin)
{
	const std::string_view sv = metadata.lookup.position_of(origin);
	const parser::text_range r = metadata.lines.text_range_for(sv);
	return r.first.line_number;
}

void color_line_by_origin(
	lang::position_tag origin,
	const parser::parse_metadata& metadata,
	std::vector<ImU32>& line_colors,
	ImU32 color)
{
	if (!lang::is_valid(origin))
		return;

	const std::size_t line_number = origins_line_number(metadata, origin);
	FS_ASSERT(line_number < line_colors.size());
	line_colors[line_number] = color;
}

void color_line_by_condition_result(
	const std::optional<lang::condition_match_result>& opt_result,
	const parser::parse_metadata& metadata,
	std::vector<ImU32>& line_colors)
{
	if (!opt_result)
		return;

	const lang::condition_match_result& result = *opt_result;
	const lang::position_tag origin = result.condition_origin();

	if (!lang::is_valid(origin))
		return;

	const std::size_t line_number = origins_line_number(metadata, origin);
	FS_ASSERT(line_number < line_colors.size());

	if (result.is_successful())
		line_colors[line_number] = color_background_condition_success;
	else
		line_colors[line_number] = color_background_condition_failure;
}

void color_line_by_condition_result(
	const lang::range_condition_match_result& result,
	const parser::parse_metadata& metadata,
	std::vector<ImU32>& line_colors)
{
	/*
	 * Range conditions that are defined using = will have the same origin for both bounds
	 * (= x is treated as >= x and <= x at the same time). We need to avoid coloring the
	 * same line twice - otherwise one result with override another.
	 *
	 * If origins are the same, the coloring must use success color only if both succeded.
	 */
	if (result.lower_bound.has_value() && result.upper_bound.has_value()) {
		if (lang::compare((*result.lower_bound).condition_origin(), (*result.upper_bound).condition_origin()) == 0) {

			const bool is_success = (*result.lower_bound).is_successful() && (*result.upper_bound).is_successful();
			const lang::position_tag origin = (*result.lower_bound).condition_origin();

			if (is_success)
				color_line_by_origin(origin, metadata, line_colors, color_background_condition_success);
			else
				color_line_by_origin(origin, metadata, line_colors, color_background_condition_failure);

			return;
		}
	}

	color_line_by_condition_result(result.lower_bound, metadata, line_colors);
	color_line_by_condition_result(result.upper_bound, metadata, line_colors);
}

template <typename Action>
void color_line_by_action_origin(
	const Action& action,
	const parser::parse_metadata& metadata,
	std::vector<ImU32>& line_colors)
{
	color_line_by_origin(action.origin, metadata, line_colors, color_background_matched_action);
}

template <typename Action>
void color_line_by_action_origin(
	const std::optional<Action>& opt_action,
	const parser::parse_metadata& metadata,
	std::vector<ImU32>& line_colors)
{
	if (!opt_action)
		return;

	color_line_by_action_origin(*opt_action, metadata, line_colors);
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
	const gui::font_settings& fonting,
	float offset_y)
{
	const auto _1 = fonting.scoped_monospaced_font();

	const auto first_column_width = get_item_tooltip_first_column_width(fonting);
	const auto second_column_width = get_item_tooltip_second_column_width(itm, fonting);

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

void debug_state::recompute(const parser::parse_metadata& metadata)
{
	if (!_info)
		return;

	for (auto& color : _line_colors)
		color = color_background_default;
	_line_colors.resize(metadata.lines.num_lines(), color_background_default);

	const debug_info& info = *_info;
	// uncomment if item_filter is actually used within debug_state class
	// FS_ASSERT(info.result.get().match_history.size() <= info.filter.get().blocks.size());

	const lang::item_filtering_result& result = info.result.get();
	for (const lang::block_match_result& block : result.match_history) {
		/*
		 * Right now every condition is colored in the same way.
		 * Might want to do more precise coloring later
		 * (then specific condition results will have specific types).
		 */
		const lang::condition_set_match_result& cs = block.condition_set_result;
		color_line_by_condition_result(cs.item_level,               metadata, _line_colors);
		color_line_by_condition_result(cs.drop_level,               metadata, _line_colors);
		color_line_by_condition_result(cs.quality,                  metadata, _line_colors);
		color_line_by_condition_result(cs.rarity,                   metadata, _line_colors);
		color_line_by_condition_result(cs.linked_sockets,           metadata, _line_colors);
		color_line_by_condition_result(cs.height,                   metadata, _line_colors);
		color_line_by_condition_result(cs.width,                    metadata, _line_colors);
		color_line_by_condition_result(cs.stack_size,               metadata, _line_colors);
		color_line_by_condition_result(cs.gem_level,                metadata, _line_colors);
		color_line_by_condition_result(cs.map_tier,                 metadata, _line_colors);
		color_line_by_condition_result(cs.area_level,               metadata, _line_colors);
		color_line_by_condition_result(cs.corrupted_mods,           metadata, _line_colors);
		color_line_by_condition_result(cs.class_,                   metadata, _line_colors);
		color_line_by_condition_result(cs.base_type,                metadata, _line_colors);
		color_line_by_condition_result(cs.sockets,                  metadata, _line_colors);
		color_line_by_condition_result(cs.socket_group,             metadata, _line_colors);
		color_line_by_condition_result(cs.has_explicit_mod,         metadata, _line_colors);
		color_line_by_condition_result(cs.has_enchantment,          metadata, _line_colors);
		color_line_by_condition_result(cs.prophecy,                 metadata, _line_colors);
		color_line_by_condition_result(cs.enchantment_passive_node, metadata, _line_colors);
		color_line_by_condition_result(cs.enchantment_passive_num,  metadata, _line_colors);
		color_line_by_condition_result(cs.has_influence,            metadata, _line_colors);
		color_line_by_condition_result(cs.gem_quality_type,         metadata, _line_colors);
		color_line_by_condition_result(cs.is_identified,            metadata, _line_colors);
		color_line_by_condition_result(cs.is_corrupted,             metadata, _line_colors);
		color_line_by_condition_result(cs.is_mirrored,              metadata, _line_colors);
		color_line_by_condition_result(cs.is_elder_item,            metadata, _line_colors);
		color_line_by_condition_result(cs.is_shaper_item,           metadata, _line_colors);
		color_line_by_condition_result(cs.is_fractured_item,        metadata, _line_colors);
		color_line_by_condition_result(cs.is_synthesised_item,      metadata, _line_colors);
		color_line_by_condition_result(cs.is_enchanted,             metadata, _line_colors);
		color_line_by_condition_result(cs.is_shaped_map,            metadata, _line_colors);
		color_line_by_condition_result(cs.is_elder_map,             metadata, _line_colors);
		color_line_by_condition_result(cs.is_blighted_map,          metadata, _line_colors);
		color_line_by_condition_result(cs.is_replica,               metadata, _line_colors);
		color_line_by_condition_result(cs.is_alternate_quality,     metadata, _line_colors);

		if (block.continue_origin) {
			const std::size_t line_number = origins_line_number(metadata, *block.continue_origin);
			FS_ASSERT(line_number < _line_colors.size());
			_line_colors[line_number] = color_background_matched_continue;
		}
	}

	const lang::item_style& style = result.style;
	color_line_by_action_origin(style.border_color,      metadata, _line_colors);
	color_line_by_action_origin(style.text_color,        metadata, _line_colors);
	color_line_by_action_origin(style.background_color,  metadata, _line_colors);
	color_line_by_action_origin(style.font_size,         metadata, _line_colors);
	color_line_by_action_origin(style.play_alert_sound,  metadata, _line_colors);
	color_line_by_action_origin(style.switch_drop_sound, metadata, _line_colors);
	color_line_by_action_origin(style.minimap_icon,      metadata, _line_colors);
	color_line_by_action_origin(style.play_effect,       metadata, _line_colors);

	color_line_by_origin(style.visibility.origin, metadata, _line_colors, color_background_matched_visibility);
}

void debug_state::draw_interface(const font_settings& fonting, const parser::parse_metadata* metadata)
{
	if (_popup_pending) {
		ImGui::OpenPopup(debug_popup_title);
		_popup_pending = false;
		_popup_open = true;
	}

	if (!_info || !metadata)
		return;

	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize); // take whole space
	if (ImGui::BeginPopupModal(debug_popup_title, &_popup_open, ImGuiWindowFlags_NoResize)) {
		const debug_info& info = *_info;
		_drawing_offset_y = draw_debug_interface_impl(
			info.itm.get(), info.result.get(), metadata->lines, _line_colors, fonting, _drawing_offset_y);
		ImGui::EndPopup();
	}
}

}
