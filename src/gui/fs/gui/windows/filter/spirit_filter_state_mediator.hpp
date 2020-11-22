#pragma once

#include <fs/gui/windows/filter/spirit_filter_state_mediator_base.hpp>
#include <fs/gui/windows/filter/filter_state_mediator.hpp>

namespace fs::gui {

class spirit_filter_state_mediator
	: public virtual spirit_filter_state_mediator_base
	, public filter_state_mediator
{
public:
	spirit_filter_state_mediator(application& app)
	: filter_state_mediator(app)
	{
	}

	void on_source_change(const std::optional<std::string>& source) override;
	void on_parsed_spirit_filter_change(
		const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter) override;
	void on_spirit_filter_symbols_change(
		const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter,
		const std::optional<lang::symbol_table>& spirit_filter_symbols) override;
	void on_spirit_filter_change(
		const std::optional<lang::spirit_item_filter>& spirit_filter) override;

	const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter() const override
	{
		return _parsed_spirit_filter;
	}

	const std::optional<lang::symbol_table>& spirit_filter_symbols() const override
	{
		return _spirit_filter_symbols;
	}

	const std::optional<lang::spirit_item_filter>& spirit_filter() const override
	{
		return _spirit_filter;
	}

private:
	void new_parsed_spirit_filter(std::optional<parser::parsed_spirit_filter> parsed_spirit_filter);
	void new_spirit_filter_symbols(std::optional<lang::symbol_table> spirit_filter_symbols);
	void new_spirit_filter(std::optional<lang::spirit_item_filter> spirit_filter);

	void draw_interface_derived() override;
	void draw_interface_parsed_spirit_filter();
	void draw_interface_spirit_filter_symbols();
	void draw_interface_spirit_filter();

	std::optional<parser::parsed_spirit_filter> _parsed_spirit_filter;
	std::optional<lang::symbol_table> _spirit_filter_symbols;
	std::optional<lang::spirit_item_filter> _spirit_filter;
};

}
