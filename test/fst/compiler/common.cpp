#include "fst/compiler/common.hpp"

#include "fs/compiler/compiler.hpp"
#include "fs/log/buffered_logger.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

namespace fsut::compiler
{

fs::parser::parse_data compiler_fixture::parse(std::string_view input)
{
	fs::log::buffered_logger log_buf;
	std::optional<fs::parser::parse_data> parse_result = fs::parser::parse(input, log_buf);
	const auto log_data = log_buf.flush_out(); // needs to be here, side effects in BOOST_TEST* macros do not work
	BOOST_TEST_REQUIRE(parse_result.has_value(), "parse of:\n" << input << "\nfailed:\n" << log_data);
	return *parse_result;
}

std::variant<fs::lang::constants_map, fs::compiler::error::error_variant>
compiler_fixture::resolve_constants(
	const std::vector<fs::parser::ast::constant_definition>& defs)
{
	return fs::compiler::resolve_constants(defs, fs::itemdata::item_price_data{});
}

}
