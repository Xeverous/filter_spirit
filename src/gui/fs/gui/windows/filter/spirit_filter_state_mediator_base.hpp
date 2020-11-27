#pragma once

#include <fs/gui/windows/filter/filter_state_mediator_base.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/log/thread_safe_logger.hpp>
#include <fs/log/buffer_logger.hpp>

#include <memory>

namespace fs::lang {

// symbol_table can not be forward-declared because it is a type alias
class spirit_item_filter;

	namespace market {

		class item_price_report;

	}

}

namespace fs::parser {

class parsed_spirit_filter;

}

namespace fs::gui {

class spirit_filter_state_mediator_base : public virtual filter_state_mediator_base
{
public:
	virtual std::shared_ptr<log::thread_safe_logger<log::buffer_logger>> share_logger() = 0;

	virtual void on_parsed_spirit_filter_change(
		const parser::parsed_spirit_filter* parsed_spirit_filter) = 0;

	virtual void on_spirit_filter_symbols_change(
		const parser::parsed_spirit_filter* parsed_spirit_filter,
		const lang::symbol_table* spirit_filter_symbols) = 0;

	virtual void on_spirit_filter_change(
		const lang::spirit_item_filter* spirit_filter) = 0;

	virtual void on_price_report_change(
		const lang::market::item_price_report& report) = 0;

	virtual const parser::parsed_spirit_filter* parsed_spirit_filter() const = 0;
	virtual const lang::symbol_table* spirit_filter_symbols() const = 0;
	virtual const lang::spirit_item_filter* spirit_filter() const = 0;
	virtual const lang::market::item_price_report& price_report() const = 0;
};

}
