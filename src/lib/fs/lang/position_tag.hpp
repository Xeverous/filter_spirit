#pragma once

#include <fs/utility/assert.hpp>

#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>

namespace fs::lang {

using position_tag = boost::spirit::x3::position_tagged;

inline position_tag no_origin() { return {}; }

inline position_tag merge_origins(position_tag first, position_tag last)
{
	FS_ASSERT(first.id_last <= last.id_first);
	return position_tag{ first.id_first, last.id_last };
}

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

constexpr bool is_valid(position_tag tag) noexcept
{
	return tag.id_first != -1 && tag.id_last != -1;
}

}
