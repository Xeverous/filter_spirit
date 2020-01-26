#pragma once

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string_view>

namespace fst
{

[[nodiscard]]
std::string_view search(std::string_view input, std::string_view pattern);

[[nodiscard]]
boost::test_tools::predicate_result compare_ranges(
	std::string_view expected,
	std::string_view actual,
	std::string_view whole_input /* <- for pretty-printing calculation only */);

// TODO improve logs for endline mismatches
[[nodiscard]]
boost::test_tools::predicate_result compare_strings(
	std::string_view expected,
	std::string_view actual);

}
