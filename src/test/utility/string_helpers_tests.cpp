#include <fs/utility/string_helpers.hpp>

#include <boost/test/unit_test.hpp>

class string_helpers_fixture
{
	// nothing for now
};

namespace tt = boost::test_tools;

BOOST_FIXTURE_TEST_SUITE(string_helpers_suite, string_helpers_fixture)

	BOOST_AUTO_TEST_CASE(compare_strings_ignore_diacritics)
	{
		using fs::utility::compare_strings_ignore_diacritics;

		// empty strings should always match
		BOOST_TEST(compare_strings_ignore_diacritics("", "", false));
		BOOST_TEST(compare_strings_ignore_diacritics("", "", true));

		// empty strings should match if no exatch match is required
		BOOST_TEST( compare_strings_ignore_diacritics(" ", "", false));
		BOOST_TEST(!compare_strings_ignore_diacritics(" ", "", true));

		// requirement should never match a shorter string
		BOOST_TEST(!compare_strings_ignore_diacritics("", " ", false));
		BOOST_TEST(!compare_strings_ignore_diacritics("", " ", true));

		// exact match
		BOOST_TEST(compare_strings_ignore_diacritics("The Wolf's Legacy", "The Wolf's Legacy", false));
		BOOST_TEST(compare_strings_ignore_diacritics("The Wolf's Legacy", "The Wolf's Legacy", true));

		// partial match - only with support enabled
		BOOST_TEST( compare_strings_ignore_diacritics("The Wolf's Legacy", "The Wolf", false));
		BOOST_TEST(!compare_strings_ignore_diacritics("The Wolf's Legacy", "The Wolf", true));

		// unicode - exact match
		BOOST_TEST(compare_strings_ignore_diacritics("Maelström Staff", "Maelström Staff", false));
		BOOST_TEST(compare_strings_ignore_diacritics("Maelström Staff", "Maelström Staff", true));

		// unicode - partial match
		BOOST_TEST( compare_strings_ignore_diacritics("Maelström Staff", "Maelström", false));
		BOOST_TEST(!compare_strings_ignore_diacritics("Maelström Staff", "Maelström", true));
		BOOST_TEST( compare_strings_ignore_diacritics("Maelström Staff", "ström", false));
		BOOST_TEST(!compare_strings_ignore_diacritics("Maelström Staff", "ström", true));

		// corner cases - incorrect second UTF-8 byte
		BOOST_TEST(!compare_strings_ignore_diacritics("ö", "Ö", false));
		BOOST_TEST(!compare_strings_ignore_diacritics("ö", "Ö", true));

		// corner cases - item's property character mismatch
		BOOST_TEST(!compare_strings_ignore_diacritics("o", "ö", false));
		BOOST_TEST(!compare_strings_ignore_diacritics("o", "ö", true));

		// corner cases - invalid UTF-8
		constexpr auto c = static_cast<char>(0xC3);
		const std::string_view invalid_utf8(&c, 1u);
		// the first test may pass because it is a partial match
		BOOST_TEST( compare_strings_ignore_diacritics("ö", invalid_utf8, false));
		BOOST_TEST(!compare_strings_ignore_diacritics("ö", invalid_utf8, true));
		BOOST_TEST(!compare_strings_ignore_diacritics("o", invalid_utf8, false));
		BOOST_TEST(!compare_strings_ignore_diacritics("o", invalid_utf8, true));
		BOOST_TEST(!compare_strings_ignore_diacritics(invalid_utf8, "ö", false));
		BOOST_TEST(!compare_strings_ignore_diacritics(invalid_utf8, "ö", true));
		BOOST_TEST(!compare_strings_ignore_diacritics(invalid_utf8, "o", false));
		BOOST_TEST(!compare_strings_ignore_diacritics(invalid_utf8, "o", true));
	}

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
