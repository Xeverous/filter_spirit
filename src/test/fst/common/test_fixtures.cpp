#include <fst/common/test_fixtures.hpp>

#include <fs/lang/item_price_data.hpp>
#include <fs/parser/parser.hpp>
#include <fs/compiler/resolve_symbols.hpp>
#include <fs/log/buffered_logger.hpp>
#include <fs/version.hpp>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <utility>

namespace fst
{

const std::string& minimal_input()
{
	namespace v = fs::version;
	static const std::string str =
		"version: " + std::to_string(v::major) + "." + std::to_string(v::minor) + "." + std::to_string(v::patch) + "\n"
		"config: {}";
	return str;
}

namespace pr = fs::parser;

pr::parse_success_data parser_fixture::parse(std::string_view input)
{
	std::variant<pr::parse_success_data, pr::parse_failure_data> parse_result = pr::parse(input);

	if (std::holds_alternative<pr::parse_failure_data>(parse_result))
	{
		const auto& errors = std::get<pr::parse_failure_data>(parse_result);
		fs::log::buffered_logger logger;
		pr::print_parse_errors(errors, logger);
		const auto log_data = logger.flush_out(); // needs to be here, side effects in BOOST_TEST etc macros do not work
		BOOST_FAIL("parse of:\n" << input << "\nfailed:\n" << log_data);
	}

	return std::get<pr::parse_success_data>(std::move(parse_result));
}

std::variant<fs::lang::symbol_table, fs::compiler::compile_error>
compiler_fixture::resolve_symbols(
	const std::vector<pr::ast::definition>& defs)
{
	return fs::compiler::resolve_symbols(defs, fs::lang::item_price_data{});
}

}
