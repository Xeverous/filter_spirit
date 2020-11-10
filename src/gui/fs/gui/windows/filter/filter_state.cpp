#include <fs/gui/windows/filter/filter_state.hpp>
#include <fs/parser/parser.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/generator/make_item_filter.hpp>

#include <utility>

namespace fs::gui {

namespace detail {

void filter_state_base::new_source(std::string source, log::logger& logger)
{
	_source = std::move(source);
	parse_source(logger);
}

}

void real_filter_state::parse_real_filter(log::logger& logger)
{
	if (source()) {
		std::variant<parser::parsed_real_filter, parser::parse_failure_data> result = parser::parse_real_filter(*source());

		if (std::holds_alternative<parser::parse_failure_data>(result)) {
			parser::print_parse_errors(std::get<parser::parse_failure_data>(result), logger);
			_parsed_real_filter = std::nullopt;
		}
		else {
			_parsed_real_filter = std::move(std::get<parser::parsed_real_filter>(result));
		}
	}
	else {
		_parsed_real_filter = std::nullopt;
	}

	recompute_filter_representation(logger);
}

void real_filter_state::recompute_filter_representation(log::logger& logger)
{
	if (_parsed_real_filter) {
		compiler::outcome<lang::item_filter> result = compiler::compile_real_filter({}, (*_parsed_real_filter).ast);
		compiler::output_logs(result.logs(), (*_parsed_real_filter).lookup, logger);

		if (result.has_result()) {
			filter_representation() = std::move(result.result());
		}
		else {
			filter_representation() = std::nullopt;
		}
	}
	else {
		filter_representation() = std::nullopt;
	}
}

void spirit_filter_state::parse_spirit_filter(log::logger& logger)
{
	if (source()) {
		std::variant<parser::parsed_spirit_filter, parser::parse_failure_data> result = parser::parse_spirit_filter(*source());

		if (std::holds_alternative<parser::parse_failure_data>(result)) {
			parser::print_parse_errors(std::get<parser::parse_failure_data>(result), logger);
			_parsed_spirit_filter = std::nullopt;
		}
		else {
			_parsed_spirit_filter = std::move(std::get<parser::parsed_spirit_filter>(result));
		}
	}
	else {
		_parsed_spirit_filter = std::nullopt;
	}

	resolve_spirit_filter_symbols(logger);
}

void spirit_filter_state::resolve_spirit_filter_symbols(log::logger& logger)
{
	if (_parsed_spirit_filter) {
		compiler::outcome<lang::symbol_table> result = compiler::resolve_spirit_filter_symbols(
			{}, (*_parsed_spirit_filter).ast.definitions);

		compiler::output_logs(result.logs(), (*_parsed_spirit_filter).lookup, logger);

		if (result.has_result())
			_spirit_filter_symbols = std::move(result.result());
		else
			_spirit_filter_symbols = std::nullopt;
	}
	else {
		_spirit_filter_symbols = std::nullopt;
	}

	compile_spirit_filter(logger);
}

void spirit_filter_state::compile_spirit_filter(log::logger& logger)
{
	if (_spirit_filter_symbols && _parsed_spirit_filter) {
		compiler::outcome<lang::spirit_item_filter> result = compiler::compile_spirit_filter_statements(
			{}, (*_parsed_spirit_filter).ast.statements, *_spirit_filter_symbols);

		compiler::output_logs(result.logs(), (*_parsed_spirit_filter).lookup, logger);

		if (result.has_result())
			_spirit_filter = std::move(result.result());
		else
			_spirit_filter = std::nullopt;
	}
	else {
		_spirit_filter = std::nullopt;
	}

	recompute_filter_representation(logger);
}

void spirit_filter_state::recompute_filter_representation(log::logger& /* logger */)
{
	if (_spirit_filter) {
		filter_representation() = generator::make_item_filter(*_spirit_filter, {});
	}
	else {
		filter_representation() = std::nullopt;
	}
}

}
