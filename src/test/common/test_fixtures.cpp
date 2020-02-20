#include "common/test_fixtures.hpp"

#include <fs/parser/parser.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/log/string_logger.hpp>

#include <boost/test/unit_test.hpp>

#include <utility>

namespace fs::test
{

const std::string& minimal_input()
{
	// anything that is always required in the filter
	// currently nothing
	static const std::string str;
	return str;
}

namespace pr = fs::parser;

pr::sf::parse_success_data parser_fixture::parse(std::string_view input)
{
	std::variant<pr::sf::parse_success_data, pr::parse_failure_data> parse_result = pr::sf::parse(input);

	if (std::holds_alternative<pr::parse_failure_data>(parse_result)) {
		const auto& errors = std::get<pr::parse_failure_data>(parse_result);
		fs::log::string_logger logger;
		pr::print_parse_errors(errors, logger);
		BOOST_FAIL("parse of:\n" << input << "\nfailed:\n" << logger.str());
	}

	return std::get<pr::sf::parse_success_data>(std::move(parse_result));
}

}
