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
	filter_state_mediator(const fonting& f);

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

	log::logger& logger() override
	{
		return _logger.logger();
	}

	void draw_interface(application& app) override;

protected:
	virtual void draw_interface_derived() = 0; // template method pattern

	void new_filter_representation(std::optional<lang::item_filter> filter_representation);

private:
	void draw_interface_filter_representation();
	void draw_interface_logs();
	void draw_interface_loot(application& app);

	source_state _source; // first step
	// << possible intermediate data in derived types >>
	std::optional<lang::item_filter> _filter_representation;
	loot_state _loot_state;
	debug_state _debug_state;
	gui_logger _logger;
};

}
