#pragma once

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

namespace fs::lang {

using position_tag = boost::spirit::x3::position_tagged;

constexpr int compare(position_tag lhs, position_tag rhs) noexcept
{
	if (lhs.id_first < rhs.id_first)
		return -1;

	if (lhs.id_first > rhs.id_first)
		return 1;

	if (lhs.id_last < rhs.id_last)
		return -1;

	if (lhs.id_last > rhs.id_last)
		return 1;

	return 0;
}

}
