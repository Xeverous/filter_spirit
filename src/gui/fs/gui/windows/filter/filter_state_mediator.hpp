#pragma once

#include <fs/gui/windows/filter/filter_state_mediator_base.hpp>
#include <fs/gui/windows/filter/source_state.hpp>
#include <fs/gui/windows/filter/loot_state.hpp>
#include <fs/gui/windows/filter/debug_state.hpp>
#include <fs/gui/gui_logger.hpp>
#include <fs/lang/item_filter.hpp>

#include <optional>
#include <string>

namespace fs::gui {

class fonting;

class filter_state_mediator : public virtual filter_state_mediator_base
{
public:
	void load_source_file(std::string path);
	void open_text_input();

	void on_filter_representation_change(const lang::item_filter* filter_representation) override;
	void on_debug_open(const lang::item& itm, const lang::item_filtering_result& result) override;
	void on_loot_change() override;
	void on_filter_results_change() override;
	void on_filter_results_clear() override;

	const std::string* source() const override
	{
		return _source.source();
	}

	const std::string* source_path() const override
	{
		return _source.source_path();
	}

	const lang::item_filter* filter_representation() const override
	{
		return _filter_representation.has_value() ? &*_filter_representation : nullptr;
	}

	void draw_interface(application& app) override;

protected:
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
