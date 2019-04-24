#define BOOST_TEST_MODULE Filter Spirit
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

/*
 * We do not write main function - it's already provided by
 * the Boost Test library. Add only a sanity check that everything works.
 */
BOOST_AUTO_TEST_CASE(sanity_check)
{
	BOOST_TEST(true);
}
