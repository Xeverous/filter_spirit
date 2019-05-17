#include "fst/common/test_fixtures.hpp"

#include "fs/generator/generate_filter.hpp"
#include "fs/log/buffered_logger.hpp"
#include "fs/lang/item_price_data.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string>
#include <string_view>

namespace ut = boost::unit_test;
namespace tt = boost::test_tools;

std::string generate_filter(
	std::string_view input,
	const fs::lang::item_price_data& ipd = {})
{
	fs::log::buffered_logger logger;
	std::optional<std::string> filter = fs::generator::generate_filter_without_preamble(input, ipd, fs::generator::options{}, logger);
	const auto log_data = logger.flush_out();
	BOOST_TEST_REQUIRE(filter.has_value(), "filter generation failed:\n" << log_data);
	return *filter;
}

namespace fst
{

BOOST_AUTO_TEST_SUITE(compiler_suite)

	BOOST_AUTO_TEST_CASE(minimal_input_generate_filter)
	{
		const std::string filter = generate_filter(minimal_input());
		BOOST_TEST(filter.empty());
	}

	BOOST_AUTO_TEST_SUITE(compiler_filter_generation_suite, * ut::depends_on("compiler_suite/minimal_input_generate_filter"))

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
