#pragma once

#include <fs/gui/windows/filter/filter_state_mediator_base.hpp>
#include <fs/parser/parser.hpp>
#include <fs/lang/symbol_table.hpp>

#include <optional>

namespace fs::gui {

class spirit_filter_state_mediator_base : public virtual filter_state_mediator_base
{
public:
	virtual void on_parsed_spirit_filter_change(
		const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter) = 0;

	virtual void on_spirit_filter_symbols_change(
		const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter,
		const std::optional<lang::symbol_table>& spirit_filter_symbols) = 0;

	virtual void on_spirit_filter_change(
		const std::optional<lang::spirit_item_filter>& spirit_filter) = 0;

	virtual const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter() const = 0;
	virtual const std::optional<lang::symbol_table>& spirit_filter_symbols() const = 0;
	virtual const std::optional<lang::spirit_item_filter>& spirit_filter() const = 0;
};

}
