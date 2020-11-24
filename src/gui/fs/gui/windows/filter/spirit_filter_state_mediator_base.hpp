#pragma once

#include <fs/gui/windows/filter/filter_state_mediator_base.hpp>
#include <fs/lang/symbol_table.hpp>

namespace fs::lang {

// symbol_table can not be forward-declared because it is a type alias
class spirit_item_filter;

}

namespace fs::parser {

class parsed_spirit_filter;

}

namespace fs::gui {

class spirit_filter_state_mediator_base : public virtual filter_state_mediator_base
{
public:
	virtual void on_parsed_spirit_filter_change(
		const parser::parsed_spirit_filter* parsed_spirit_filter) = 0;

	virtual void on_spirit_filter_symbols_change(
		const parser::parsed_spirit_filter* parsed_spirit_filter,
		const lang::symbol_table* spirit_filter_symbols) = 0;

	virtual void on_spirit_filter_change(
		const lang::spirit_item_filter* spirit_filter) = 0;

	virtual const parser::parsed_spirit_filter* parsed_spirit_filter() const = 0;
	virtual const lang::symbol_table* spirit_filter_symbols() const = 0;
	virtual const lang::spirit_item_filter* spirit_filter() const = 0;
};

}
