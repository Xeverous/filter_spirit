#pragma once

#include <fs/log/logger.hpp>

#include <string>

namespace fs::lang {

class item;
class item_filter;
class item_filtering_result;

}

namespace fs::parser {

class lookup_data;
class line_lookup;

}

namespace fs::gui {

class application;

class filter_state_mediator_base
{
public:
	virtual ~filter_state_mediator_base() = default;

	virtual void on_source_change(const std::string* source) = 0;
	virtual void on_filter_representation_change(const lang::item_filter* filter_representation) = 0;
	virtual void on_debug_open(const lang::item& itm, const lang::item_filtering_result& result) = 0;
	virtual void on_loot_change() = 0;
	virtual void on_filter_results_change() = 0;
	virtual void on_filter_results_clear() = 0;

	virtual const std::string* source() const = 0;
	virtual const std::string* source_path() const = 0;
	virtual const parser::lookup_data* lookup_data() const = 0;
	virtual const parser::line_lookup* line_lookup() const = 0;
	virtual const lang::item_filter* filter_representation() const = 0;

	virtual log::logger& logger() = 0;

	virtual void draw_interface(application& app) = 0;
};

}
