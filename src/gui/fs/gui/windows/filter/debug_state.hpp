#pragma once

#include <optional>

namespace fs::parser {

class lookup_data;
class line_lookup;

}

namespace fs::lang {

class item;
class item_filtering_result;

}

namespace fs::gui {

class fonting;

struct debug_info
{
	const parser::lookup_data* lookup = nullptr;
	const parser::line_lookup* lines = nullptr;
	const lang::item* itm = nullptr;
	const lang::item_filtering_result* result = nullptr;
};

class debug_state
{
public:
	void open_debug(
		const parser::lookup_data& lookup,
		const parser::line_lookup& lines,
		const lang::item& itm,
		const lang::item_filtering_result& result)
	{
		_info = debug_info{&lookup, &lines, &itm, &result};
		recompute();
		open_popup();
	}

	void open_popup()
	{
		_popup_pending = true;
	}

	void close_debug()
	{
		invalidate();
		close_popup();
	}

	void close_popup()
	{
		_popup_open = false;
	}

	void invalidate()
	{
		_info = std::nullopt;
	}

	void recompute();

	void draw_interface(const fonting& f);

private:
	std::optional<debug_info> _info;
	bool _popup_pending = false;
	bool _popup_open = false;
};

}
