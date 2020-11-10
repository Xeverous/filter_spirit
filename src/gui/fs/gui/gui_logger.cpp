#include <fs/gui/gui_logger.hpp>

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

void gui_logger::draw(float lines_of_text)
{
	const auto size_y = [&]() -> float {
		// if size is positive, then that much space is occupied
		// if size is negative, then all possible space except size space is occupied
		// (occupied space uses monospaced font)
		if (lines_of_text >= 0.0f)
			return _fonting.get().monospaced_font_size() * lines_of_text;
		else
			return ImGui::GetFontSize() * lines_of_text;
	}();

	if (ImGui::BeginChild("logs", ImVec2(0.0f, size_y), true, ImGuiWindowFlags_HorizontalScrollbar)) {
		const auto _1 = _fonting.get().scoped_monospaced_font();
		const auto _2 = scoped_text_color_override(IM_COL32(255, 255, 255, 255));

		auto last_severity = fs::log::severity::info;

		for (const auto& message : logger().messages()) {
			if (message.s != last_severity) {
				ImGui::PopStyleColor();
				ImGui::PushStyleColor(ImGuiCol_Text, severity_to_color(message.s));
				last_severity = message.s;
			}

			ImGui::TextUnformatted(message.text.c_str());
		}
	}

	ImGui::EndChild();

	if (ImGui::Button("clear logs"))
		logger().clear();
	ImGui::SameLine();
	if (ImGui::Button("copy logs"))
		copy_to_clipboard(logger().messages());
}

}
