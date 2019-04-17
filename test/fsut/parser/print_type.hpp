#pragma once

#include "fs/lang/types.hpp"

#include <ostream>
#include <type_traits>

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
namespace fs::lang
{

std::ostream& boost_test_print_type(std::ostream& os, action_type at)
{
	return os << "(action_type: " << static_cast<int>(at) << ")";
}

std::ostream& boost_test_print_type(std::ostream& os, string_condition_property scp)
{
	return os << "(string_condition_property: " << static_cast<int>(scp) << ")";
}

}
