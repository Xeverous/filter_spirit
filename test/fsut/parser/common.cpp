#include "fsut/parser/common.hpp"
#include "fs/parser/parser.hpp"
#include "fs/core/version.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

namespace fsut::parser
{

const std::string& minimal_input()
{
	namespace v = fs::core::version;
	static const std::string str =
		"version: " + std::to_string(v::major) + "." + std::to_string(v::minor) + "." + std::to_string(v::patch) + "\n"
		"config: {}";
	return str;
}

fs::parser::ast::ast_type parser_fixture::parse(std::string_view input)
{
	const std::optional<fs::parser::parse_data> parse_result = fs::parser::parse(input, log_buf);
	BOOST_TEST_REQUIRE(parse_result.has_value(), "parse of:\n" << input << "\nfailed\n" << log_buf.flush_out());
	return (*parse_result).ast;
}

}
