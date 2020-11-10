#pragma once

#include <fs/parser/parser.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/log/logger.hpp>

#include <optional>
#include <string>
#include <utility>

namespace fs::gui {

class real_filter_state
{
public:
	real_filter_state() = default;
	real_filter_state(std::string source, log::logger& logger)
	{
		new_source(std::move(source), logger);
	}

	void new_source(std::string source, log::logger& logger);
	void parse_real_filter(log::logger& logger);
	void recompute_filter_representation(log::logger& logger);

	const auto& parsed_real_filter() const
	{
		return _parsed_real_filter;
	}

	const auto& filter_representation() const
	{
		return _filter_representation;
	}

private:
	std::optional<std::string> _source;
	std::optional<parser::parsed_real_filter> _parsed_real_filter;
	std::optional<lang::item_filter> _filter_representation;
};

class spirit_filter_state
{
public:
	spirit_filter_state() = default;
	spirit_filter_state(std::string source, log::logger& logger)
	{
		new_source(std::move(source), logger);
	}

	void new_source(std::string source, log::logger& logger);
	void parse_spirit_filter(log::logger& logger);
	void resolve_spirit_filter_symbols(log::logger& logger);
	void compile_spirit_filter(log::logger& logger);
	void recompute_filter_representation(log::logger& logger);

	const auto& parsed_spirit_filter() const
	{
		return _parsed_spirit_filter;
	}

	const auto& spirit_filter_symbols() const
	{
		return _spirit_filter_symbols;
	}

	const auto& spirit_filter() const
	{
		return _spirit_filter;
	}

	const auto& filter_representation() const
	{
		return _filter_representation;
	}

private:
	std::optional<std::string> _source;
	std::optional<parser::parsed_spirit_filter> _parsed_spirit_filter;
	std::optional<lang::symbol_table> _spirit_filter_symbols;
	std::optional<lang::spirit_item_filter> _spirit_filter;
	std::optional<lang::item_filter> _filter_representation;
};

}
