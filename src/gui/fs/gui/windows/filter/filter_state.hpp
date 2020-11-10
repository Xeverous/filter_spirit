#pragma once

#include <fs/parser/parser.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/log/logger.hpp>

#include <optional>
#include <string>
#include <utility>

namespace fs::gui {

namespace detail {

/**
 * @class base class for filter editor/debugger functionality
 * @details This class holds only source (first step) and filter representation (last step).
 * Descentant classes are supposed to hold their data for intermediate steps and modify
 * filter representation here when needed.
 */
class filter_state_base
{
public:
	filter_state_base() = default;
	filter_state_base(std::string source, log::logger& logger)
	{
		new_source(std::move(source), logger);
	}

	virtual ~filter_state_base() = default;

	void new_source(std::string source, log::logger& logger);
	virtual void parse_source(log::logger& logger) = 0;

	const auto& source() const
	{
		return _source;
	}

	auto& filter_representation()
	{
		return _filter_representation;
	}

	const auto& filter_representation() const
	{
		return _filter_representation;
	}

private:
	std::optional<std::string> _source; // first step: source code
	std::optional<lang::item_filter> _filter_representation; // last step: debuggable filter representation
};

}

class real_filter_state : public detail::filter_state_base
{
public:
	real_filter_state() = default;
	real_filter_state(std::string source, log::logger& logger)
	: detail::filter_state_base(std::move(source), logger)
	{
	}

	void parse_source(log::logger& logger) override
	{
		parse_real_filter(logger);
	}

	void parse_real_filter(log::logger& logger);
	void recompute_filter_representation(log::logger& logger);

	const auto& parsed_real_filter() const
	{
		return _parsed_real_filter;
	}

private:
	std::optional<parser::parsed_real_filter> _parsed_real_filter;
};

class spirit_filter_state : public detail::filter_state_base
{
public:
	spirit_filter_state() = default;
	spirit_filter_state(std::string source, log::logger& logger)
	: detail::filter_state_base(std::move(source), logger)
	{
	}

	void parse_source(log::logger& logger) override
	{
		parse_spirit_filter(logger);
	}

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

private:
	std::optional<parser::parsed_spirit_filter> _parsed_spirit_filter;
	std::optional<lang::symbol_table> _spirit_filter_symbols;
	std::optional<lang::spirit_item_filter> _spirit_filter;
};

}
