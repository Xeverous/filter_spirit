#include "common/test_fixtures.hpp"

#include <fs/parser/parser.hpp>

#include <boost/test/unit_test.hpp>

namespace fs::test
{

BOOST_AUTO_TEST_SUITE(parser_suite)

	class parser_error_fixture
	{
	protected:
		/*
		 * We can not easily test parse failures - any future change in the grammar will result in very different
		 * expected tokens. So test only that the parse failed and that at least 1 error was returned.
		 */
		void expect_parse_failure(std::string str)
		{
			const std::string input_str = minimal_input() + str;
			const std::string_view input = input_str;
			std::variant<parser::parsed_spirit_filter, parser::parse_failure_data> parse_result = parser::parse_spirit_filter(input);
			BOOST_TEST_REQUIRE(std::holds_alternative<parser::parse_failure_data>(parse_result));
			const auto& parse_data = std::get<parser::parse_failure_data>(parse_result);
			BOOST_TEST(!parse_data.errors.empty());
		}
	};

	BOOST_FIXTURE_TEST_SUITE(parser_error_suite, parser_error_fixture)

		BOOST_AUTO_TEST_CASE(string_missing_close_quote)
		{
			expect_parse_failure("$str = \"abc");
		}

		BOOST_AUTO_TEST_CASE(string_with_line_break)
		{
			expect_parse_failure("$str = \"ab\nc\"");
		}

		BOOST_AUTO_TEST_CASE(too_large_integer)
		{
			expect_parse_failure("$val = 123456789123456789");
		}

		BOOST_AUTO_TEST_CASE(duplicated_eq_sign_1)
		{
			expect_parse_failure("$val == 1");
		}

		BOOST_AUTO_TEST_CASE(duplicated_eq_sign_2)
		{
			expect_parse_failure("$val = = 2");
		}

		BOOST_AUTO_TEST_CASE(missing_value)
		{
			expect_parse_failure("$x = \n $y = 2");
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
