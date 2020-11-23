#pragma once

#include <string_view>
#include <vector>

#include <imgui.h>

namespace fs::gui::aux {

inline ImVec2 measure_text_line(std::string_view line, int font_size, const ImFont* fnt)
{
	return fnt->CalcTextSizeA(font_size, FLT_MAX, 0.0f, line.data(), line.data() + line.size());
}

void on_hover_text_tooltip(std::string_view text);

bool button_positive(const char* str);
bool button_negative(const char* str);

class multiline_text_input
{
public:
	multiline_text_input()
	{
		_buf.resize(1024);
	}

	void set_text(std::string_view text);

	bool draw(ImVec2 size = {});

	const char* c_str() const
	{
		return _buf.data();
	}

private:
	// not std::string because for multi-line text string's small buffer
	// optimization is more code complexity than benefit
	std::vector<char> _buf;
};

}
