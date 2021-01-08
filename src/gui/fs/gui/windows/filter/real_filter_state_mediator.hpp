#pragma once

#include <fs/gui/windows/filter/filter_state_mediator.hpp>
#include <fs/parser/parser.hpp>
#include <fs/log/buffer_logger.hpp>

#include <optional>

namespace fs::gui {

class real_filter_state_mediator: public filter_state_mediator
{
public:
	real_filter_state_mediator() = default;

	real_filter_state_mediator(real_filter_state_mediator&& other) noexcept = default;
	real_filter_state_mediator& operator=(real_filter_state_mediator&& other) noexcept = default;

	log::buffer_logger& logger() override
	{
		return _logger;
	}

	void on_source_change(const std::string* source) override;
	void on_parsed_real_filter_change(const parser::parsed_real_filter* parsed_real_filter);

	const parser::parsed_real_filter* parsed_real_filter() const
	{
		return _parsed_real_filter.has_value() ? &*_parsed_real_filter : nullptr;
	}

	const parser::lookup_data* lookup_data() const override;
	const parser::line_lookup* line_lookup() const override;

private:
	void new_parsed_real_filter(std::optional<parser::parsed_real_filter> parsed_real_filter);

	void draw_interface_derived(application& app) override;
	void draw_interface_logs_derived(gui_logger& gl, const fonting& f) override;

	std::optional<parser::parsed_real_filter> _parsed_real_filter;
	log::buffer_logger _logger;
};

}
