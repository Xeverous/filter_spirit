#pragma once

#include <fs/gui/windows/filter/loot_state.hpp>
#include <fs/parser/parser.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/lang/symbol_table.hpp>
#include <fs/log/logger.hpp>

#include <optional>
#include <string>
#include <utility>

namespace fs::gui {

class application;

namespace detail {

/**
 * @class base class for filter editor/debugger functionality
 * @details This class implements 2 design patterns:
 * - facade: This class holds a lot of state (source, parsed source, filter, debug info, ...)
 *   that needs to be managed in specific order (ususally transforming previous-step data to next-step data).
 *   Accessors of private fields return by const reference to preserve invariants. Modification of private
 *   fields is only possible through new_* functions which trigger a chain reaction to update next fields
 *   that depend on the modified one and then further next ones until last field is reached.
 * - template method: Some of the update steps are pure virtual functions because real and spirit filters
 *   have different state to manage. Thus, the implementation of some intermediate steps is expected in derived types.
 *
 * Class invariants:
 * - There are fields A, B, C, ... X, Y, Z. Some of them are being stored in descendants of this class.
 * - If B is changed, C needs to be recomputed and then D and then ... up to Z.
 * - (N+1)th field may be dependent on any of (0...N)th fields, thus if Nth field is a null optional,
 *   all further fields should also be null optionals.
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

	void new_source(std::optional<std::string> source, log::logger& logger);
	virtual void on_source_change(const std::optional<std::string>& source, log::logger& logger) = 0;

	void new_filter_representation(std::optional<lang::item_filter> filter_representation, log::logger& logger);
	void on_filter_representation_change(const std::optional<lang::item_filter>& filter_representation, log::logger& logger);

	const auto& source() const
	{
		return _source;
	}

	const auto& filter_representation() const
	{
		return _filter_representation;
	}

	void draw_interface(application& app);

protected:
	virtual void draw_interface_derived() = 0;

private:
	void draw_interface_source();
	void draw_interface_filter_representation();
	void draw_interface_loot(application& app);

	std::optional<std::string> _source; // first step
	// << possible intermediate data in derived types >>
	std::optional<lang::item_filter> _filter_representation;
	loot_state _loot_state;
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

	void on_source_change(const std::optional<std::string>& source, log::logger& logger) override;

	void new_parsed_real_filter(std::optional<parser::parsed_real_filter> parsed_real_filter, log::logger& logger);
	void on_parsed_real_filter_change(const std::optional<parser::parsed_real_filter>& parsed_real_filter, log::logger& logger);

	const auto& parsed_real_filter() const
	{
		return _parsed_real_filter;
	}

private:
	void draw_interface_derived() override;

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

	void on_source_change(const std::optional<std::string>& source, log::logger& logger) override;

	void new_parsed_spirit_filter(std::optional<parser::parsed_spirit_filter> parsed_spirit_filter, log::logger& logger);
	void on_parsed_spirit_filter_change(const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter, log::logger& logger);

	void new_spirit_filter_symbols(std::optional<lang::symbol_table> spirit_filter_symbols, log::logger& logger);
	void on_spirit_filter_symbols_change(
		const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter,
		const std::optional<lang::symbol_table>& spirit_filter_symbols,
		log::logger& logger);

	void new_spirit_filter(std::optional<lang::spirit_item_filter> spirit_filter, log::logger& logger);
	void on_spirit_filter_change(const std::optional<lang::spirit_item_filter>& spirit_filter, log::logger& logger);

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
	void draw_interface_derived() override;

	void draw_interface_parsed_spirit_filter();
	void draw_interface_spirit_filter_symbols();
	void draw_interface_spirit_filter();

	std::optional<parser::parsed_spirit_filter> _parsed_spirit_filter;
	std::optional<lang::symbol_table> _spirit_filter_symbols;
	std::optional<lang::spirit_item_filter> _spirit_filter;
};

}
