#pragma once

#include <fs/gui/windows/filter/source_state.hpp>
#include <fs/gui/windows/filter/loot_state.hpp>
#include <fs/gui/windows/filter/debug_state.hpp>
#include <fs/gui/gui_logger.hpp>
#include <fs/lang/item_filter.hpp>

#include <optional>
#include <string>

namespace fs::gui {

class fonting;

class filter_state_mediator
{
public:
	filter_state_mediator() = default;

	virtual ~filter_state_mediator() = default;

#ifndef __EMSCRIPTEN__
	void load_source_file(std::string path);
#endif
	void open_text_input();
	void new_source(std::string source);

	virtual void on_source_change(const std::string* source) = 0; // template method pattern
	void on_filter_representation_change(const lang::item_filter* filter_representation);
	void on_debug_open(const lang::item& itm, const lang::item_filtering_result& result);
	void on_loot_change();
	void on_filter_results_change();
	void on_filter_results_clear();

	// template method pattern
	virtual log::logger& logger() = 0;

	source_state& source()
	{
		return _source;
	}

	const source_state& source() const
	{
		return _source;
	}

	// template method pattern
	virtual const parser::parse_metadata* parse_metadata() const = 0;

	const lang::item_filter* filter_representation() const
	{
		return _filter_representation.has_value() ? &*_filter_representation : nullptr;
	}

	void draw_interface(application& app);

	// template method pattern
	virtual void draw_interface_save_filter(const lang::item_filter& filter, log::logger& logger) = 0;

protected:
	filter_state_mediator(filter_state_mediator&& other) noexcept = default;
	filter_state_mediator& operator=(filter_state_mediator&& other) noexcept = default;

	// template method pattern
	virtual void draw_interface_logs_derived(gui_logger& gl, const fonting& f) = 0;
	virtual void draw_interface_derived(application& app) = 0;

	void new_filter_representation(std::optional<lang::item_filter> filter_representation);

private:
	void draw_interface_filter_representation();
	void draw_interface_logs(application& app);
	void draw_interface_loot(application& app);

	source_state _source; // first step
	// << possible intermediate data in derived types >>
	std::optional<lang::item_filter> _filter_representation;
	loot_state _loot_state;
	debug_state _debug_state;
	gui_logger _logger;
};

}
