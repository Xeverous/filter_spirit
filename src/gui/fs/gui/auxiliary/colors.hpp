#pragma once

#include <imgui.h>

namespace fs::gui::aux {

constexpr ImU32 color_true  = IM_COL32(  0, 255,   0, 255);
constexpr ImU32 color_false = IM_COL32(255,   0,   0, 255);

constexpr ImU32 color_normal_item = IM_COL32(200, 200, 200, 255);
constexpr ImU32 color_magic_item  = IM_COL32(136, 136, 255, 255);
constexpr ImU32 color_rare_item   = IM_COL32(255, 255, 119, 255);
constexpr ImU32 color_unique_item = IM_COL32(175,  96,  37, 255);

}
