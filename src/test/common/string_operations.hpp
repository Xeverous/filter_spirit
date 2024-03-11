#pragma once

#include <fs/utility/string_helpers.hpp>

#include <boost/test/unit_test.hpp>

#include <functional>
#include <string_view>

namespace fs::test
{

class search_state
{
public:
	search_state(std::string_view input, std::string_view pattern);

	std::string_view result() const
	{
		return utility::make_string_view(first, last);
	}

	search_state& next();

private:
	std::default_searcher<const char*> searcher;
	const char* first = nullptr;
	const char* last = nullptr;
};

[[nodiscard]]
search_state search(std::string_view input, std::string_view pattern);

[[nodiscard]]
boost::test_tools::predicate_result compare_ranges(
	std::string_view expected,
	std::string_view actual,
	std::string_view whole_input /* <- for pretty-printing calculation only */);

[[nodiscard]]
boost::test_tools::predicate_result compare_strings(
	std::string_view expected,
	std::string_view actual);

}
