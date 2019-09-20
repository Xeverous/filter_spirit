#include "fst/common/test_fixtures.hpp"
#include "fst/common/string_operations.hpp"

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

		BOOST_AUTO_TEST_CASE(comments)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
# a single "Show" block with a comment above
Show
## other comment
# # # something else
)");
			const std::string_view expected_filter =
R"(Show

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(single_action)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
SetTextColor RGB(1, 2, 3)
Show
)");
			const std::string_view expected_filter =
R"(Show
	SetTextColor 1 2 3

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(single_condition)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
Quality > 0 {
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	Quality > 0

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(multiple_actions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
SetTextColor RGB(1, 2, 3)
SetBackgroundColor RGB(11, 22, 33)
SetFontSize 36
Show
)");
			const std::string_view expected_filter =
R"(Show
	SetTextColor 1 2 3
	SetBackgroundColor 11 22 33
	SetFontSize 36

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(multiple_conditions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
BaseType "Vaal"
Quality > 0
Width 1 {
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	Quality > 0
	Width = 1
	BaseType "Vaal"

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(constants)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
const x = 36
const c1 = RGB(1, 2, x)

SetTextColor c1
SetBackgroundColor c1
SetFontSize x
Show
)");
			const std::string_view expected_filter =
R"(Show
	SetTextColor 1 2 36
	SetBackgroundColor 1 2 36
	SetFontSize 36

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(nested_actions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
const x = 36
const y = 38
const c1 = RGB(1, 2, x)
const c2 = RGB(11, 22, y)

BaseType "Vaal"
Width > 1 {
	SetBorderColor c2
	SetBackgroundColor c2
	SetFontSize y
	Show
}

SetTextColor c1
SetBackgroundColor c1
SetFontSize x
Show
)");
			const std::string_view expected_filter =
R"(Show
	Width > 1
	BaseType "Vaal"
	SetBorderColor 11 22 38
	SetBackgroundColor 11 22 38
	SetFontSize 38

Show
	SetTextColor 1 2 36
	SetBackgroundColor 1 2 36
	SetFontSize 36

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(nested_blocks)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
SetBackgroundColor RGB(0, 0, 0)

Class "Boots" {
	SetBorderColor RGB(1, 2, 3)

	BaseType ["Dragonscale Boots", "Sorcerer Boots"] {
		SetAlertSound 1
		Quality > 0 {
			SetBorderColor RGB(255, 255, 255)
			Show
		}

		Show
	}

	Hide
}

Show
)");
			const std::string_view expected_filter =
R"(Show
	Quality > 0
	Class "Boots"
	BaseType "Dragonscale Boots" "Sorcerer Boots"
	SetBorderColor 255 255 255
	SetBackgroundColor 0 0 0
	PlayAlertSound 1

Show
	Class "Boots"
	BaseType "Dragonscale Boots" "Sorcerer Boots"
	SetBorderColor 1 2 3
	SetBackgroundColor 0 0 0
	PlayAlertSound 1

Hide
	Class "Boots"
	SetBorderColor 1 2 3
	SetBackgroundColor 0 0 0

Show
	SetBackgroundColor 0 0 0

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(simple_price_queries)
		{
			fs::lang::item_price_data ipd;
			ipd.divination_cards.push_back(fs::lang::elementary_item{"Rain of Chaos", fs::lang::price_data{0.125, 0, 0, 0, 0, 0, 0, 0, 0, 0}});
			ipd.divination_cards.push_back(fs::lang::elementary_item{"Humility", fs::lang::price_data{5, 0, 0, 0, 0, 0, 0, 0, 0, 0}});
			ipd.divination_cards.push_back(fs::lang::elementary_item{"A Dab of Ink", fs::lang::price_data{10, 0, 0, 0, 0, 0, 0, 0, 0, 0}});
			ipd.divination_cards.push_back(fs::lang::elementary_item{"Abandoned Wealth", fs::lang::price_data{100, 0, 0, 0, 0, 0, 0, 0, 0, 0}});
			ipd.divination_cards.push_back(fs::lang::elementary_item{"The Doctor", fs::lang::price_data{1000, 0, 0, 0, 0, 0, 0, 0, 0, 0}});
			const std::string actual_filter = generate_filter(minimal_input() + R"(
const low = $divination(0, 5)

BaseType $divination(100, 999999) { Show }
BaseType $divination(10, 100) { Show }
BaseType $divination(5, 10) { Show }
BaseType low { Hide }
)", ipd);
			const std::string_view expected_filter =
R"(Show
	BaseType "Abandoned Wealth" "The Doctor"

Show
	BaseType "A Dab of Ink"

Show
	BaseType "Humility"

Hide
	BaseType "Rain of Chaos"

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
