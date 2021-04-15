#pragma once

#include <fs/gui/windows/filter/filter_state_mediator.hpp>
#include <fs/gui/windows/filter/market_data_state.hpp>
#include <fs/compiler/symbol_table.hpp>
#include <fs/parser/parser.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/log/buffer_logger.hpp>
#include <fs/log/thread_safe_logger.hpp>

#include <utility>
#include <optional>
#include <memory>

namespace fs::gui {

class spirit_filter_state_mediator: public filter_state_mediator
{
public:
	spirit_filter_state_mediator(std::vector<lang::league> available_leagues)
	: _market_data(std::move(available_leagues))
	, _logger_ptr(std::make_shared<log::thread_safe_logger<log::buffer_logger>>())
	{
	}

	spirit_filter_state_mediator(spirit_filter_state_mediator&& other) noexcept = default;
	spirit_filter_state_mediator& operator=(spirit_filter_state_mediator&& other) noexcept = default;

	log::logger& logger() override;
	void clear_logs() override;

	std::shared_ptr<log::thread_safe_logger<log::buffer_logger>> share_logger()
	{
		return _logger_ptr;
	}

	void on_source_change(const std::string* source) override;
	void on_parsed_spirit_filter_change(
		const parser::parsed_spirit_filter* parsed_spirit_filter);
	void on_spirit_filter_symbols_change(
		const parser::parsed_spirit_filter* parsed_spirit_filter,
		const compiler::symbol_table* spirit_filter_symbols);
	void on_spirit_filter_change(
		const lang::spirit_item_filter* spirit_filter);
	void on_price_report_change(
		const lang::market::item_price_report& report);

	const parser::parsed_spirit_filter* parsed_spirit_filter() const
	{
		return _parsed_spirit_filter.has_value() ? &*_parsed_spirit_filter : nullptr;
	}

	const parser::parse_metadata* parse_metadata() const override;

	const compiler::symbol_table* spirit_filter_symbols() const
	{
		return _spirit_filter_symbols.has_value() ? &*_spirit_filter_symbols : nullptr;
	}

	const lang::spirit_item_filter* spirit_filter() const
	{
		return _spirit_filter.has_value() ? &*_spirit_filter : nullptr;
	}

	const lang::market::item_price_report& price_report() const
	{
		return _market_data.price_report();
	}

private:
	void new_parsed_spirit_filter(std::optional<parser::parsed_spirit_filter> parsed_spirit_filter);
	void new_spirit_filter_symbols(std::optional<compiler::symbol_table> spirit_filter_symbols);
	void new_spirit_filter(std::optional<lang::spirit_item_filter> spirit_filter);

	void refresh_filter_representation(
		const lang::spirit_item_filter* spirit_filter,
		const lang::market::item_price_data& item_price_data);

	void draw_interface_derived(application& app) override;
	void draw_interface_save_filter(const lang::item_filter& filter, log::logger& logger) override;
	void draw_interface_logs_derived(gui_logger& gl, const fonting& f) override;

	void draw_interface_parsed_spirit_filter();
	void draw_interface_spirit_filter_symbols();
	void draw_interface_spirit_filter();

	std::optional<parser::parsed_spirit_filter> _parsed_spirit_filter;
	std::optional<compiler::symbol_table> _spirit_filter_symbols;
	std::optional<lang::spirit_item_filter> _spirit_filter;
	market_data_state _market_data;
	std::shared_ptr<log::thread_safe_logger<log::buffer_logger>> _logger_ptr;
};

}
