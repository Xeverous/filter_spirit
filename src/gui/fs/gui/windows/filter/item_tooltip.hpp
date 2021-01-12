#pragma once

namespace fs::lang {

struct item;
struct item_filtering_result;

}

namespace fs::gui {

class fonting;

float get_item_tooltip_first_column_width(const gui::fonting& f);
float get_item_tooltip_second_column_width(const lang::item& itm, const gui::fonting& f);

void draw_item_tooltip_first_column();
void draw_item_tooltip_second_column(const lang::item& itm);

void draw_item_tooltip(const lang::item& itm, const lang::item_filtering_result& /* result */, const gui::fonting& f);

}
