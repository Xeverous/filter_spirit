#include <fs/utility/string_helpers.hpp>

#include <boost/test/unit_test.hpp>

class string_helpers_fixture
{
	// nothing for now
};

namespace tt = boost::test_tools;

BOOST_FIXTURE_TEST_SUITE(string_helpers_suite, string_helpers_fixture)

	class trim_fixture
	{
	protected:
		void test_ltrim(std::string_view input, char c, std::string_view output)
		{
			BOOST_TEST(fs::utility::ltrim(input, c) == output, tt::per_element());
		}

		void test_rtrim(std::string_view input, char c, std::string_view output)
		{
			BOOST_TEST(fs::utility::rtrim(input, c) == output, tt::per_element());
		}

		void test_trim(std::string_view input, char c, std::string_view output)
		{
			BOOST_TEST(fs::utility::trim(input, c) == output, tt::per_element());
		}
	};

	BOOST_FIXTURE_TEST_SUITE(trim_suite, trim_fixture)

		BOOST_AUTO_TEST_CASE(empty_input)
		{
			test_ltrim("", ' ', "");
			test_rtrim("", ' ', "");
			test_trim ("", ' ', "");
		}

		BOOST_AUTO_TEST_CASE(typical_input)
		{
			test_ltrim("   A B   ", ' ', "A B   ");
			test_rtrim("   A B   ", ' ', "   A B");
			test_trim ("   A B   ", ' ', "A B");
		}

		BOOST_AUTO_TEST_CASE(only_space_input)
		{
			test_ltrim("      ", ' ', "");
			test_rtrim("      ", ' ', "");
			test_trim ("      ", ' ', "");
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
