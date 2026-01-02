#pragma once

#include <imgui.h>

namespace fs::gui::aux {

struct rect
{
	ImVec2 top_left;
	ImVec2 size;

	bool contains(ImVec2 point) const
	{
		return top_left.x <= point.x
			&& point.x < top_left.x + size.x
			&& top_left.y <= point.y
			&& point.y < top_left.y + size.y;
	}
};

}
