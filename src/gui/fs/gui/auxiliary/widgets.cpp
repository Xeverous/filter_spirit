#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/gui/auxiliary/raii.hpp>
#include <fs/utility/assert.hpp>

#include <imgui.h>

#include <cstring>

namespace {

int input_text_callback(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		auto& buf = *reinterpret_cast<std::vector<char>*>(data->UserData);
		buf.resize(data->BufTextLen * 2);
		data->Buf = buf.data(); // this is necessary: resize could invalidate data pointer
	}

	return 0;
}

int input_string_callback(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		auto& buf = *reinterpret_cast<std::string*>(data->UserData);
		buf.resize(data->BufTextLen * 2);
		data->Buf = buf.data(); // this is necessary: resize could invalidate data pointer
	}

	return 0;
}

bool button_colored(const char* str, ImU32 color, ImU32 color_hovered, ImU32 color_active)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertU32ToFloat4(color));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::ColorConvertU32ToFloat4(color_hovered));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::ColorConvertU32ToFloat4(color_active));
	bool result = ImGui::Button(str);
	ImGui::PopStyleColor(3);
	return result;
}

}

namespace fs::gui::aux {

void on_hover_text_tooltip(std::string_view text)
{
	FS_ASSERT(!text.empty()); // Dear ImGui does not work with empty ranges

	if (ImGui::IsItemHovered())
	{
		scoped_tooltip _;
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(text.data(), text.data() + text.size());
		ImGui::PopTextWrapPos();
	}
}

bool button_positive(const char* str)
{
	return button_colored(str, IM_COL32(  0, 127,   0, 255), IM_COL32(  0, 192,   0, 255), IM_COL32(  0, 255,   0, 255));
}

bool button_negative(const char* str)
{
	return button_colored(str, IM_COL32(127,   0,   0, 255), IM_COL32(192,   0,   0, 255), IM_COL32(255,   0,   0, 255));
}

bool multiline_text_input::draw(ImVec2 size)
{
	return ImGui::InputTextMultiline(
		"", _buf.data(), _buf.size(), size,
		ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize,
		&input_text_callback, &_buf);
}

void multiline_text_input::set_text(std::string_view text)
{
	if (_buf.size() <= text.size())
		_buf.resize(text.size() + 1);

	std::memcpy(_buf.data(), text.data(), text.size());
	_buf[text.size()] = '\0';
}

bool input_string(const char* label, std::string& str)
{
	str.resize(str.capacity()); // add extra space filled with '\0' bytes

	bool result = ImGui::InputText(
		label, str.data(), str.size(),
		ImGuiInputTextFlags_CallbackResize, &input_string_callback, &str);

	str.resize(std::strlen(str.c_str())); // remove extra '\0' bytes
	return result;
}

}
