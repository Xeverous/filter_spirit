#pragma once

#include "fs/parser/detail/config.hpp"

#include <boost/range/iterator_range_core.hpp>

#include <string_view>
#include <type_traits>

namespace fs::parser::detail
{

inline
std::string_view to_string_view(range_type range)
{
	static_assert(
		std::is_same_v<range_type, boost::iterator_range<std::string_view::const_iterator>>,
		"parser's range_type changed, please update implementation of this function");

	// corner case for empty ranges must be handled separately
	// only default-constructed string view is empty
	if (range.empty())
		return std::string_view();

	const auto first = range.begin();
	const auto last  = range.end();
	// surprise: std::string_view does not have a ctor that takes its own iterators!
	// but ... since std::string_view iterators guuarantee continuous storage
	// we can use std::string_view(const char*, size_type) ctor
	return std::string_view(&*first, last - first);
}

}
