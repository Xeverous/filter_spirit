#pragma once

#include <fs/lang/object.hpp>
#include <fs/lang/action_properties.hpp>
#include <fs/lang/condition_properties.hpp>

#include <boost/test/unit_test.hpp>

#include <ostream>
#include <optional>

/*
 * Boost Test requires all types to be printable, otherwise
 * BOOST_TEST(a == b) will not compile. It is possible to workaround
 * it by BOOST_TEST((a == b)) but this will hide information and just
 * print the boolean result
 *
 * https://www.boost.org/doc/libs/1_70_0/libs/test/doc/html/boost_test/test_output/test_tools_support_for_logging/testing_tool_output_disable.html
 *
 * there are 3 solutions:
 *
 * - BOOST_TEST_DONT_PRINT_LOG_VALUE(type)
 * - overload std::ostream& operator<<(std::ostream&, type) in type's namespace
 * - overload std::ostream& boost_test_print_type(std::ostream&, type) in type's namespace
 *
 * Note: ADL will not work if type is defined in a namespace and overload is in global scope
 * Note: the last option can not be a template because boost's implementation already is and doing so results in ambiguous calls
 */
BOOST_TEST_DONT_PRINT_LOG_VALUE(std::optional<int>)

namespace fs::lang
{

inline
std::ostream& boost_test_print_type(std::ostream& os, suit_type s)
{
	return os << "(suit: " << static_cast<int>(s) << ")";
}

inline
std::ostream& boost_test_print_type(std::ostream& os, object_type type)
{
	return os << "(object_type: " << to_string_view(type) << ")";
}

inline
std::ostream& boost_test_print_type(std::ostream& os, std::optional<object_type> type)
{
	if (type.has_value())
		return boost_test_print_type(os, *type);
	else
		return os << "(object_type: ?)";
}

}
