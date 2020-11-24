#include <fs/gui/windows/filter/debug_state.hpp>
#include <fs/gui/windows/filter/item_tooltip.hpp>
#include <fs/gui/settings/fonting.hpp>
#include <fs/parser/parser.hpp>
#include <fs/lang/item.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/utility/assert.hpp>

#include <imgui.h>

namespace {

using namespace fs;

void draw_debug_interface_impl(
	std::string_view source,
	const lang::item& itm,
	const lang::item_filtering_result& /* result */,
	const gui::fonting& f)
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

	if (ImGui::BeginChild("source")) {
		ImGui::TextUnformatted(source.data(), source.data() + source.size());
	}
	ImGui::EndChild();

	ImGui::Columns(1); // reset back to single column
}

constexpr auto debug_popup_title = "Item debug";

}

namespace fs::gui {

void debug_state::recompute()
{
	// TODO implement actual debug
	FS_ASSERT(_info.has_value());
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

	ImGui::SetNextWindowSize({f.monospaced_font_size() * 60.0f, 0.0f});
	if (ImGui::BeginPopupModal(debug_popup_title, &_popup_open)) {
		// TODO implement actual debug
		const debug_info& info = *_info;
		draw_debug_interface_impl(info.lookup.get().get_view_of_whole_content(), info.itm.get(), info.result.get(), f);
		ImGui::EndPopup();
	}
}

}
