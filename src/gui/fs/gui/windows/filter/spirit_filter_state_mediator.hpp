#pragma once

#include <fs/gui/windows/filter/spirit_filter_state_mediator_base.hpp>
#include <fs/gui/windows/filter/filter_state_mediator.hpp>
#include <fs/parser/parser.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/symbol_table.hpp>

#include <optional>

namespace fs::gui {

class spirit_filter_state_mediator
	: public virtual spirit_filter_state_mediator_base
	, public filter_state_mediator
{
public:
	spirit_filter_state_mediator(const fonting& f)
	: filter_state_mediator(f)
	{
	}

	void on_source_change(const std::string* source) override;
	void on_parsed_spirit_filter_change(
		const parser::parsed_spirit_filter* parsed_spirit_filter) override;
	void on_spirit_filter_symbols_change(
		const parser::parsed_spirit_filter* parsed_spirit_filter,
		const lang::symbol_table* spirit_filter_symbols) override;
	void on_spirit_filter_change(
		const lang::spirit_item_filter* spirit_filter) override;

	const parser::parsed_spirit_filter* parsed_spirit_filter() const override
	{
		return _parsed_spirit_filter.has_value() ? &*_parsed_spirit_filter : nullptr;
	}

	const parser::lookup_data* lookup_data() const override;
	const parser::line_lookup* line_lookup() const override;

	const lang::symbol_table* spirit_filter_symbols() const override
	{
		return _spirit_filter_symbols.has_value() ? &*_spirit_filter_symbols : nullptr;
	}

	const lang::spirit_item_filter* spirit_filter() const override
	{
		return _spirit_filter.has_value() ? &*_spirit_filter : nullptr;
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
