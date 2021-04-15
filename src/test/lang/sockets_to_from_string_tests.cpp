#include <fs/lang/item.hpp>

#include <boost/test/unit_test.hpp>

#include <string>
#include <string_view>
#include <algorithm>

namespace ut = boost::unit_test;
namespace tt = boost::test_tools;

namespace {

using namespace fs;

bool compare_sockets(lang::socket_info lhs, lang::socket_info rhs)
{
	/*
	 * note: items R R-G-B and R-G-B R are functionally identical,
	 * but we do not sort the link-groups or anything like that,
	 * we compare strictly group by group (with socket by socket
	 * in each group) because the representation preserves order
	 * of colors/links and so the to/from string functions should too.
	 */
	return std::equal(
		lhs.groups.begin(), lhs.groups.end(),
		rhs.groups.begin(), rhs.groups.end(),
		[](lang::linked_sockets lhs, lang::linked_sockets rhs) {
			return std::equal(
				lhs.sockets.begin(), lhs.sockets.end(),
				rhs.sockets.begin(), rhs.sockets.end());
		});
}

void test_sockets_to_from_string(std::string_view str, lang::socket_info sockets)
{
	BOOST_TEST_REQUIRE(sockets.is_valid());
	BOOST_TEST(lang::to_string(sockets) == str, tt::per_element());
	std::optional<lang::socket_info> maybe_sockets = lang::to_socket_info(str);
	BOOST_TEST_REQUIRE(maybe_sockets.has_value());
	BOOST_TEST(compare_sockets(maybe_sockets.value(), sockets));
}

}

namespace fs::test {



} // namspace fs::test
