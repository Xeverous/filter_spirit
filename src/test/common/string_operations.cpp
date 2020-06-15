#include "common/string_operations.hpp"

#include <fs/utility/string_helpers.hpp>

#include <string_view>
#include <utility>
#include <tuple>

namespace
{

void find_and_print_mismatched_line(
	std::string_view expected,
	std::string_view actual,
	boost::wrap_stringstream& ss)
{
	const auto expected_first = expected.data();
	const auto expected_last  = expected.data() + expected.size();
	const auto actual_first = actual.data();
	const auto actual_last  = actual.data() + actual.size();
	const auto [expected_it, actual_it] = std::mismatch(expected_first, expected_last, actual_first, actual_last);

	// add 1 because line numbering starts at 1
	const auto line_number = std::count(actual_first, actual_it, '\n') + 1;
	ss << "on line " << line_number << " (expected/actual):\n";

	const auto print_line = [&ss](auto first, auto pos, auto last)
	{
		const auto line_first = std::find(std::make_reverse_iterator(pos), std::make_reverse_iterator(first), '\n').base();
		const auto line_last  = std::find(pos, last, '\n');

		for (auto it = line_first; it != line_last; ++it)
			ss << *it;

		ss << '\n';

		for (auto it = line_first; it != pos; ++it)
			ss << (*it == '\t' ? '\t' : ' ');

		ss << "^\n";
	};

	print_line(expected_first, expected_it, expected_last);
	print_line(actual_first, actual_it, actual_last);
}

}

namespace fs::test
{

search_state::search_state(std::string_view input, std::string_view pattern)
: searcher(pattern.data(), pattern.data() + pattern.size())
, first(input.data())
, last(input.data() + input.size())
{
	next();
}

search_state& search_state::next()
{
	std::tie(first, last) = searcher(first, last);
	return *this;
}

search_state search(std::string_view input, std::string_view pattern)
{
	BOOST_TEST_REQUIRE(!input.empty(), "test is written incorrectly, input is empty");
	BOOST_TEST_REQUIRE(!pattern.empty(), "test is written incorrectly, pattern is empty");

	/*
	 * We use this function to search within input for the text that causes compiler error.
	 * Since we always want to get that text in order to compare it with error description,
	 * a failed search should be flagged as internal test error.
	 */
	auto ss = search_state(input, pattern);
	BOOST_TEST_REQUIRE(!ss.result().empty(),
		"test is written incorrectly, search within tested input code failed\n"
		"input:\n" << input << "\npattern:\n" << pattern);
	return ss;
}

boost::test_tools::predicate_result compare_ranges(
	std::string_view expected,
	std::string_view actual,
	std::string_view whole_input)
{
	if (expected.empty())
		BOOST_FAIL("test written incorrectly: an empty range should never be expected");

	if (expected == actual)
		return true;

	boost::test_tools::predicate_result result(false);
	result.message()
		<< "EXPECTED:\n" << fs::utility::range_underline_to_string(whole_input, expected)
		<< "ACTUAL:\n" << fs::utility::range_underline_to_string(whole_input, actual);
	return result;
}

boost::test_tools::predicate_result compare_strings(
	std::string_view expected,
	std::string_view actual)
{
	if (expected == actual)
		return true;

	boost::test_tools::predicate_result result(false);
	BOOST_TEST(expected.size() == actual.size());
	find_and_print_mismatched_line(expected, actual, result.message());
	return result;
}

}
