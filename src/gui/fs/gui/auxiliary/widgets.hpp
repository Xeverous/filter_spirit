#pragma once

#include <string_view>

#include <imgui.h>

namespace fs::gui::aux {

inline ImVec2 measure_text_line(std::string_view line, int font_size, const ImFont* fnt)
{
	return fnt->CalcTextSizeA(font_size, FLT_MAX, 0.0f, line.data(), line.data() + line.size());
}

void on_hover_text_tooltip(std::string_view text);

}
