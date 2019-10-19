#include <fst/common/test_fixtures.hpp>

#include <fs/parser/parser.hpp>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace fs;

namespace fst
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
			std::variant<parser::parse_success_data, parser::parse_failure_data> parse_result = parser::parse(input);
			BOOST_TEST_REQUIRE(std::holds_alternative<parser::parse_failure_data>(parse_result));
			const auto& parse_data = std::get<parser::parse_failure_data>(parse_result);
			BOOST_TEST(!parse_data.errors.empty());
		}
	};

	BOOST_FIXTURE_TEST_SUITE(parser_error_suite, parser_error_fixture)

		BOOST_AUTO_TEST_CASE(string_missing_close_quote)
		{
			expect_parse_failure("str = \"abc");
		}

		BOOST_AUTO_TEST_CASE(array_missing_close_bracket)
		{
			expect_parse_failure("arr = [1, 2, 3, 4");
		}

		BOOST_AUTO_TEST_CASE(array_missing_open_bracket)
		{
			expect_parse_failure("arr = 1, 2, 3, 4]");
		}

		BOOST_AUTO_TEST_CASE(array_missing_delimeter)
		{
			expect_parse_failure("arr = [1, 2, 3 4]");
		}

		BOOST_AUTO_TEST_CASE(array_missing_element)
		{
			expect_parse_failure("arr = [1, 2, , 4]");
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
