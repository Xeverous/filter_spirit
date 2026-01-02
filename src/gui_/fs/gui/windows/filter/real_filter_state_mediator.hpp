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

	void clear_logs() override
	{
		_logger.clear();
	}

	void on_source_change(const std::string* source) override;
	void on_parsed_real_filter_change(const parser::parsed_real_filter* parsed_real_filter);

	const parser::parsed_real_filter* parsed_real_filter() const
	{
		return _parsed_real_filter.has_value() ? &*_parsed_real_filter : nullptr;
	}

	const parser::parse_metadata* parse_metadata() const override;

private:
	void new_parsed_real_filter(std::optional<parser::parsed_real_filter> parsed_real_filter);

	void draw_interface_derived(const network_settings& networking, network::cache& cache) override;
	void draw_interface_save_filter(const lang::item_filter& /* filter */, log::logger& /* logger */) override {}
	void draw_interface_logs_derived(gui_logger& gl, const font_settings& fonting) override;

	std::optional<parser::parsed_real_filter> _parsed_real_filter;
	log::buffer_logger _logger;
};

}
