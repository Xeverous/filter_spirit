#pragma once

#include <fs/gui/windows/filter/source_state.hpp>
#include <fs/gui/windows/filter/loot_state.hpp>
#include <fs/gui/windows/filter/debug_state.hpp>
#include <fs/gui/gui_logger.hpp>
#include <fs/lang/item_filter.hpp>

#include <optional>
#include <string>

namespace fs::lang::loot {

struct item_database;
class generator;

}

namespace fs::network { struct cache; }

namespace fs::gui {

class font_settings;
class network_settings;
struct gui_settings;

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
	virtual void clear_logs() = 0;

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

	void draw(
		const gui_settings& settings,
		const lang::loot::item_database& db,
		lang::loot::generator& gen,
		network::cache& network_cache);

	// template method pattern
	virtual void draw_interface_save_filter(const lang::item_filter& filter, log::logger& logger) = 0;

protected:
	filter_state_mediator(filter_state_mediator&& other) noexcept = default;
	filter_state_mediator& operator=(filter_state_mediator&& other) noexcept = default;

	// template method pattern
	virtual void draw_interface_logs_derived(gui_logger& gl, const font_settings& fonting) = 0;
	virtual void draw_interface_derived(const network_settings& networking, network::cache& cache) = 0;

	void new_filter_representation(std::optional<lang::item_filter> filter_representation);

private:
	void draw_interface_filter_representation();
	void draw_interface_logs(const font_settings& fonting);
	void draw_interface_loot(
		const font_settings& fonting,
		const lang::loot::item_database& db,
		lang::loot::generator& gen);

	source_state _source; // first step
	// << possible intermediate data in derived types >>
	std::optional<lang::item_filter> _filter_representation;
	loot_state _loot_state;
	debug_state _debug_state;
	gui_logger _logger;
};

}
