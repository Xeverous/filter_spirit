#pragma once

namespace fs::gui {

class theming
{
public:
	void draw_theme_selection_ui();

	int num_themes() const noexcept;
	void apply_current_theme() const;
	const char* current_theme_name() const noexcept;
	int current_theme_index() const noexcept
	{
		return _current_theme_index;
	}

private:
	int _current_theme_index = 3;
};

}
