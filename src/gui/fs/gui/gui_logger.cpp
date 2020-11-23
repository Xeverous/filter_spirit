#include <fs/gui/gui_logger.hpp>
#include <fs/gui/auxiliary/raii.hpp>

#include <imgui.h>

#include <numeric>

namespace {

ImU32 severity_to_color(fs::log::severity s)
{
	if (s == fs::log::severity::info)
		return IM_COL32(255, 255, 255, 255);
	else if (s == fs::log::severity::warning)
		return IM_COL32(255, 255,   0, 255);
	else if (s == fs::log::severity::error)
		return IM_COL32(255,   0,   0, 255);

	return IM_COL32(255, 255, 255, 255);
}

void copy_to_clipboard(const std::vector<fs::log::buffer_logger::message>& messages)
{
	const auto length = std::accumulate(messages.begin(), messages.end(), static_cast<std::size_t>(0),
		[](std::size_t sum, const auto& message) {
			return sum + message.text.size();
		}
	);
	std::string result;
	result.reserve(length);

	for (const auto& message : messages)
		result += message.text;

	ImGui::SetClipboardText(result.c_str());
}

}

namespace fs::gui {

void gui_logger::clear_logs()
{
	logger().clear();
	_last_log_size = 0;
}

void gui_logger::draw()
{
	if (ImGui::Button("clear logs"))
		clear_logs();
	ImGui::SameLine();
	if (ImGui::Button("copy logs"))
		copy_to_clipboard(logger().messages());

	if (ImGui::BeginChild("logs", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar)) {
		const auto _1 = _fonting.get().scoped_monospaced_font();
		const auto _2 = aux::scoped_text_color_override(IM_COL32(255, 255, 255, 255));

		auto last_severity = log::severity::info;

		for (const auto& message : logger().messages()) {
			if (message.s != last_severity) {
				ImGui::PopStyleColor();
				ImGui::PushStyleColor(ImGuiCol_Text, severity_to_color(message.s));
				last_severity = message.s;
			}

			ImGui::TextUnformatted(message.text.c_str());
		}

		// auto-scroll to bottom if logs size changed
		if (_last_log_size < logger().messages().size()) {
			ImGui::SetScrollHereY(1.0f);
			_last_log_size = logger().messages().size();
		}
	}

	ImGui::EndChild();
}

}
