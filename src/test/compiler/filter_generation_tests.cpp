#include "common/test_fixtures.hpp"
#include "common/string_operations.hpp"

#include <fs/compiler/compiler.hpp>
#include <fs/log/string_logger.hpp>
#include <fs/lang/market/item_price_data.hpp>

#include <boost/test/unit_test.hpp>

#include <string>
#include <string_view>

namespace ut = boost::unit_test;

namespace {

std::string generate_filter(
	std::string_view input,
	const fs::lang::market::item_price_data& ipd = {})
{
	fs::log::string_logger logger;
	std::optional<std::string> filter = fs::compiler::parse_compile_generate_spirit_filter_without_preamble(
		input, ipd, fs::compiler::settings{}, logger);
	BOOST_TEST_REQUIRE(filter.has_value(), "filter generation failed:\n" << logger.str());
	return *filter;
}

}

namespace fs::test
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
SetTextColor 1 2 3
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
Quality > 0
{
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
SetTextColor 1 2 3
SetBorderColor 11 22 33
SetBackgroundColor 111 222 233
SetFontSize 36
PlayEffect Yellow
MinimapIcon 2 Cyan UpsideDownHouse
CustomAlertSoundOptional "ding.wav"
EnableDropSound
DisableDropSoundIfAlertSound
Show
)");
			const std::string_view expected_filter =
R"(Show
	SetTextColor 1 2 3
	SetBorderColor 11 22 33
	SetBackgroundColor 111 222 233
	SetFontSize 36
	PlayEffect Yellow
	MinimapIcon 2 Cyan UpsideDownHouse
	CustomAlertSoundOptional "ding.wav"
	EnableDropSound
	DisableDropSoundIfAlertSound

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(multiple_conditions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
BaseType "Vaal"
Quality > 0
Width 1
EnchantmentPassiveNum <= 9
{
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	BaseType "Vaal"
	Quality > 0
	Width 1
	EnchantmentPassiveNum <= 9

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(range_condition)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
ItemLevel >= 75
{
	SetTextColor 1 1 1
	Show
	Continue
}

ItemLevel >= 60
ItemLevel < 75
{
	SetTextColor 2 2 2
	Show
	Continue
}

ItemLevel < 60
{
	SetTextColor 3 3 3
	Show
	Continue
}
)");
			const std::string_view expected_filter =
R"(Show
	ItemLevel >= 75
	SetTextColor 1 1 1
	Continue

Show
	ItemLevel >= 60
	ItemLevel < 75
	SetTextColor 2 2 2
	Continue

Show
	ItemLevel < 60
	SetTextColor 3 3 3
	Continue

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(range_condition_min_max)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
ItemLevel >= 85 75 80
{
	SetTextColor 1 1 1
	Show
	Continue
}

ItemLevel >= 65 60 70
ItemLevel < 70 75 65
{
	SetTextColor 2 2 2
	Show
	Continue
}
)");
			const std::string_view expected_filter =
R"(Show
	ItemLevel >= 75
	SetTextColor 1 1 1
	Continue

Show
	ItemLevel >= 60
	ItemLevel < 75
	SetTextColor 2 2 2
	Continue

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(strings_condition)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$x = None

Class "Divination Card"
{
	# Nones should be skipped
	BaseType == "The Wolf" $x "The Demon" None
	{
		Show
	}
	BaseType    "The Wolf" $x "The Demon" None
	{
		Show
	}

	# these blocks should not be generated (only Nones)
	BaseType == None
	{
		Show
	}
	BaseType    None
	{
		Show
	}
	BaseType    $x
	{
		Show
	}
}

EnchantmentPassiveNode    "Damage while you have a Herald"
{
	Show
}
EnchantmentPassiveNode =  "Damage while you have a Herald"
{
	Show
}
EnchantmentPassiveNode == "Damage while you have a Herald"
{
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	Class "Divination Card"
	BaseType == "The Wolf" "The Demon"

Show
	Class "Divination Card"
	BaseType "The Wolf" "The Demon"

Show
	EnchantmentPassiveNode "Damage while you have a Herald"

Show
	EnchantmentPassiveNode "Damage while you have a Herald"

Show
	EnchantmentPassiveNode == "Damage while you have a Herald"

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(influences_condition)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
BaseType "Leather Belt"
{
	HasInfluence    None
	{
		Show
	}
	HasInfluence == Crusader Hunter
	{
		Show
	}
	HasInfluence    Redeemer Warlord
	{
		Show
	}
}
)");
			const std::string_view expected_filter =
R"(Show
	BaseType "Leather Belt"
	HasInfluence None

Show
	BaseType "Leather Belt"
	HasInfluence == Crusader Hunter

Show
	BaseType "Leather Belt"
	HasInfluence Redeemer Warlord

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(socket_spec_conditions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
Sockets        1 2 3R AA 3D 5GBW
{
	Show
}
Sockets     =  1 2 3R AA 3D 5GBW
{
	Show
}
Sockets     == 1 2 3R AA 3D 5GBW
{
	Show
}
Sockets     >= 1 2 3R AA 3D 5GBW
{
	Show
}
SocketGroup    1 2 3R AA 3D 5GBW
{
	Show
}
SocketGroup =  1 2 3R AA 3D 5GBW
{
	Show
}
SocketGroup == 1 2 3R AA 3D 5GBW
{
	Show
}
SocketGroup >= 1 2 3R AA 3D 5GBW
{
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	Sockets 1 2 3R AA 3D 5GBW

Show
	Sockets 1 2 3R AA 3D 5GBW

Show
	Sockets == 1 2 3R AA 3D 5GBW

Show
	Sockets >= 1 2 3R AA 3D 5GBW

Show
	SocketGroup 1 2 3R AA 3D 5GBW

Show
	SocketGroup 1 2 3R AA 3D 5GBW

Show
	SocketGroup == 1 2 3R AA 3D 5GBW

Show
	SocketGroup >= 1 2 3R AA 3D 5GBW

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}


		BOOST_AUTO_TEST_CASE(constants)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$x = 36
$c1 = 1 2 $x

SetTextColor $c1
SetBackgroundColor $c1
SetFontSize $x
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

		BOOST_AUTO_TEST_CASE(expand_actions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$x = 1 2 3
$actions =
{
	SetBorderColor $x
	SetTextColor $x
}

$actions_copy =
{
	Expand $actions
}

SetFontSize 36
Expand $actions_copy
Rarity Rare
{
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	Rarity Rare
	SetTextColor 1 2 3
	SetBorderColor 1 2 3
	SetFontSize 36

)";
			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(expand_in_expand)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$x = 11 22 33
$y = 1 2 3

$actions1 =
{
	SetBorderColor $x
	SetTextColor $x
}

$actions2 =
{
	SetBorderColor $y
	Expand $actions1
	SetTextColor $y
}

SetFontSize 36
Expand $actions2
Rarity Rare
{
	Show
}

Show
)");
			const std::string_view expected_filter =
R"(Show
	Rarity Rare
	SetTextColor 1 2 3
	SetBorderColor 11 22 33
	SetFontSize 36

Show
	SetTextColor 1 2 3
	SetBorderColor 11 22 33
	SetFontSize 36

)";
			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(expand_in_expand_override)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$x = 11 22 33
$y = 1 2 3

$actions1 =
{
	SetBorderColor $x
	SetTextColor $x
	SetFontSize 42
}

$actions2 =
{
	SetBorderColor $y
	SetTextColor $y
}

SetFontSize 36
Expand $actions2
Rarity Rare
{
	Expand $actions1

	Quality 20
	{
		SetBackgroundColor 50 50 50
		Expand $actions2
		SetTextColor 100 100 100
		Show
	}

	Show
}

Show
)");
			const std::string_view expected_filter =
R"(Show
	Rarity Rare
	Quality 20
	SetTextColor 100 100 100
	SetBorderColor 1 2 3
	SetBackgroundColor 50 50 50
	SetFontSize 42

Show
	Rarity Rare
	SetTextColor 11 22 33
	SetBorderColor 11 22 33
	SetFontSize 42

Show
	SetTextColor 1 2 3
	SetBorderColor 1 2 3
	SetFontSize 36

)";
			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(expand_nested_tree)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$color_chromatic_small = 175 255 200
$color_chromatic_big   = 128 128 192

$chrome_item_logic =
{
	SocketGroup RGB
	{
		SetBorderColor $color_chromatic_small

		Width 2
		Height <= 2
		{
			Show
		}

		Width 1
		Height <= 4
		{
			Show
		}

		SetBorderColor $color_chromatic_big
		Show
	}

	Show
}

Rarity Normal
{
	SetTextColor 200 200 200
	Expand $chrome_item_logic
}

Rarity Magic
{
	SetTextColor 136 136 255
	Expand $chrome_item_logic
}

Rarity Rare
{
	SetTextColor 255 255 119
	Expand $chrome_item_logic
}
)");
			const std::string_view expected_filter =
R"(Show
	Rarity Normal
	SocketGroup RGB
	Width 2
	Height <= 2
	SetTextColor 200 200 200
	SetBorderColor 175 255 200

Show
	Rarity Normal
	SocketGroup RGB
	Width 1
	Height <= 4
	SetTextColor 200 200 200
	SetBorderColor 175 255 200

Show
	Rarity Normal
	SocketGroup RGB
	SetTextColor 200 200 200
	SetBorderColor 128 128 192

Show
	Rarity Normal
	SetTextColor 200 200 200

Show
	Rarity Magic
	SocketGroup RGB
	Width 2
	Height <= 2
	SetTextColor 136 136 255
	SetBorderColor 175 255 200

Show
	Rarity Magic
	SocketGroup RGB
	Width 1
	Height <= 4
	SetTextColor 136 136 255
	SetBorderColor 175 255 200

Show
	Rarity Magic
	SocketGroup RGB
	SetTextColor 136 136 255
	SetBorderColor 128 128 192

Show
	Rarity Magic
	SetTextColor 136 136 255

Show
	Rarity Rare
	SocketGroup RGB
	Width 2
	Height <= 2
	SetTextColor 255 255 119
	SetBorderColor 175 255 200

Show
	Rarity Rare
	SocketGroup RGB
	Width 1
	Height <= 4
	SetTextColor 255 255 119
	SetBorderColor 175 255 200

Show
	Rarity Rare
	SocketGroup RGB
	SetTextColor 255 255 119
	SetBorderColor 128 128 192

Show
	Rarity Rare
	SetTextColor 255 255 119

)";
			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(nested_actions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$x = 36
$y = 38
$c1 = 1 2 $x
$c2 = 11 22 $y

BaseType "Vaal"
Width > 1
{
	SetBorderColor $c2
	SetBackgroundColor $c2
	SetFontSize $y
	Show
}

SetTextColor $c1
SetBackgroundColor $c1
SetFontSize $x
Show
)");
			const std::string_view expected_filter =
R"(Show
	BaseType "Vaal"
	Width > 1
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
SetBackgroundColor 0 0 0

Class "Boots"
{
	SetBorderColor 1 2 3

	BaseType "Dragonscale Boots" "Sorcerer Boots"
	{
		PlayAlertSound 1
		Quality > 0
		{
			SetBorderColor 255 255 255
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
	Class "Boots"
	BaseType "Dragonscale Boots" "Sorcerer Boots"
	Quality > 0
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

		BOOST_AUTO_TEST_CASE(action_override)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
PlayAlertSoundPositional 1 300
Show

Class "Boots"
{
	SetBorderColor 1 2 3
	CustomAlertSound "pop.wav"
	Hide
}

SetBackgroundColor 255 0 0
Show
)");
			const std::string_view expected_filter =
R"(Show
	PlayAlertSoundPositional 1 300

Hide
	Class "Boots"
	SetBorderColor 1 2 3
	CustomAlertSound "pop.wav"

Show
	SetBackgroundColor 255 0 0
	PlayAlertSoundPositional 1 300

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(dynamic_visibility_enabled)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$show_rares = True
$show_hammers = True

BaseType "Gavel" "Stone Hammer" "Rock Breaker"
{
	SetTextColor 255 255 255
	SetBackgroundColor 162 85 0

	Rarity Normal
	{
		 ShowDiscard $show_hammers
	}

	Rarity Magic
	Quality > 12
	{
		 ShowDiscard $show_hammers
	}

	Rarity Rare
	Quality > 16
	{
		 ShowDiscard $show_hammers
	}
}

Rarity Rare
{
	ShowHide $show_rares
}
)");
			const std::string_view expected_filter =
R"(Show
	BaseType "Gavel" "Stone Hammer" "Rock Breaker"
	Rarity Normal
	SetTextColor 255 255 255
	SetBackgroundColor 162 85 0

Show
	BaseType "Gavel" "Stone Hammer" "Rock Breaker"
	Rarity Magic
	Quality > 12
	SetTextColor 255 255 255
	SetBackgroundColor 162 85 0

Show
	BaseType "Gavel" "Stone Hammer" "Rock Breaker"
	Rarity Rare
	Quality > 16
	SetTextColor 255 255 255
	SetBackgroundColor 162 85 0

Show
	Rarity Rare

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(dynamic_visibility_disabled)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$show_rares = True
$show_hammers = False

BaseType "Gavel" "Stone Hammer" "Rock Breaker"
{
	SetTextColor 255 255 255
	SetBackgroundColor 162 85 0

	Rarity Normal
	{
		ShowDiscard $show_hammers
	}

	Rarity Magic
	Quality > 12
	{
		ShowDiscard $show_hammers
	}

	Rarity Rare
	Quality > 16
	{
		ShowDiscard $show_hammers
	}
}

Rarity Rare
{
	ShowHide $show_rares
}
)");
			const std::string_view expected_filter =
R"(Show
	Rarity Rare

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(dynamic_visibility_multiple_values)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
$x1 = True
$x2 = True
$x3 = True
$xx = $x1 $x2 $x3

ShowHide $x1 $x2 $x3
ShowHide $x1 True $x2 True $x3
ShowHide $xx
ShowHide False $xx
ShowHide $x1 True $xx True $x2 False $x3
)");
			const std::string_view expected_filter =
R"(Show

Show

Show

Hide

Hide

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(continue_statement)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
SetFontSize 30

DropLevel >= 10
{
	SetFontSize 35

	ItemLevel < 70
	{
		Hide
		Continue
	}

	Rarity >= Rare
	ItemLevel >= 70
	{
		SetFontSize 40
		Show
		Continue
	}

	Sockets 6
	{
		PlayEffect Blue
		Show
		Continue
	}

	Show
	Continue
}

Hide
)");
			const std::string_view expected_filter =
R"(Hide
	DropLevel >= 10
	ItemLevel < 70
	SetFontSize 35
	Continue

Show
	DropLevel >= 10
	Rarity >= Rare
	ItemLevel >= 70
	SetFontSize 40
	Continue

Show
	DropLevel >= 10
	Sockets 6
	SetFontSize 35
	PlayEffect Blue
	Continue

Show
	DropLevel >= 10
	SetFontSize 35
	Continue

Hide
	SetFontSize 30

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(disabled_actions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
MinimapIcon 0 Yellow Star
PlayEffect Yellow

DropLevel >= 10
{
	MinimapIcon 1 Green Raindrop
	PlayEffect Green

	ItemLevel < 70
	{
		MinimapIcon -1
		PlayEffect None
		Hide
	}

	Rarity >= Rare
	ItemLevel >= 70
	{
		SetFontSize 40
		DisableDropSound
		Show
	}

	Sockets 6
	{
		MinimapIcon -1
		PlayEffect None
		Show
		Continue
	}

	EnableDropSound
	Show
}

Hide
)");
			const std::string_view expected_filter =
R"(Hide
	DropLevel >= 10
	ItemLevel < 70
	PlayEffect None
	MinimapIcon -1

Show
	DropLevel >= 10
	Rarity >= Rare
	ItemLevel >= 70
	SetFontSize 40
	PlayEffect Green
	MinimapIcon 1 Green Raindrop
	DisableDropSound

Show
	DropLevel >= 10
	Sockets 6
	PlayEffect None
	MinimapIcon -1
	Continue

Show
	DropLevel >= 10
	PlayEffect Green
	MinimapIcon 1 Green Raindrop
	EnableDropSound

Hide
	PlayEffect Yellow
	MinimapIcon 0 Yellow Star

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(disabled_sounds)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
SetAlertSound ShChaos 200
Show
Continue

ItemLevel 1
{
	PlayAlertSound 1 100
	Show
	Continue
}

ItemLevel 2
{
	PlayAlertSound 2
	Show
	Continue
}

ItemLevel 3
{
	PlayAlertSound ShGeneral 300
	Show
	Continue
}

ItemLevel 4
{
	PlayAlertSound ShGeneral
	Show
	Continue
}

ItemLevel 5
{
	PlayAlertSound None 50
	Show
	Continue
}

ItemLevel 6
{
	PlayAlertSound None
	Show
	Continue
}

ItemLevel 1
{
	PlayAlertSoundPositional 1 100
	Show
	Continue
}

ItemLevel 2
{
	PlayAlertSoundPositional 2
	Show
	Continue
}

ItemLevel 3
{
	PlayAlertSoundPositional ShGeneral 300
	Show
	Continue
}

ItemLevel 4
{
	PlayAlertSoundPositional ShGeneral
	Show
	Continue
}

ItemLevel 5
{
	PlayAlertSoundPositional None 50
	Show
	Continue
}

ItemLevel 6
{
	PlayAlertSoundPositional None
	Show
	Continue
}

ItemLevel 7
{
	CustomAlertSound "" 250
	Show
	Continue
}

ItemLevel 8
{
	CustomAlertSound ""
	Show
	Continue
}

ItemLevel 9
{
	CustomAlertSound "None" 150
	Show
	Continue
}

ItemLevel 10
{
	CustomAlertSound "None"
	Show
	Continue
}

ItemLevel 1
{
	SetAlertSound 1 100
	Show
	Continue
}

ItemLevel 2
{
	SetAlertSound 2
	Show
	Continue
}

ItemLevel 3
{
	SetAlertSound ShGeneral 300
	Show
	Continue
}

ItemLevel 4
{
	SetAlertSound ShGeneral
	Show
	Continue
}

ItemLevel 5
{
	SetAlertSound None 50
	Show
	Continue
}

ItemLevel 6
{
	SetAlertSound None
	Show
	Continue
}

ItemLevel 7
{
	SetAlertSound "" 250
	Show
	Continue
}

ItemLevel 8
{
	SetAlertSound ""
	Show
	Continue
}

ItemLevel 9
{
	SetAlertSound "None" 150
	Show
	Continue
}

ItemLevel 10
{
	SetAlertSound "None"
	Show
	Continue
}

PlayAlertSound ShVaal 100
Show
)");
			const std::string_view expected_filter =
R"(Show
	PlayAlertSound ShChaos 200
	Continue

Show
	ItemLevel 1
	PlayAlertSound 1 100
	Continue

Show
	ItemLevel 2
	PlayAlertSound 2
	Continue

Show
	ItemLevel 3
	PlayAlertSound ShGeneral 300
	Continue

Show
	ItemLevel 4
	PlayAlertSound ShGeneral
	Continue

Show
	ItemLevel 5
	PlayAlertSound None
	Continue

Show
	ItemLevel 6
	PlayAlertSound None
	Continue

Show
	ItemLevel 1
	PlayAlertSoundPositional 1 100
	Continue

Show
	ItemLevel 2
	PlayAlertSoundPositional 2
	Continue

Show
	ItemLevel 3
	PlayAlertSoundPositional ShGeneral 300
	Continue

Show
	ItemLevel 4
	PlayAlertSoundPositional ShGeneral
	Continue

Show
	ItemLevel 5
	PlayAlertSoundPositional None
	Continue

Show
	ItemLevel 6
	PlayAlertSoundPositional None
	Continue

Show
	ItemLevel 7
	CustomAlertSound ""
	Continue

Show
	ItemLevel 8
	CustomAlertSound ""
	Continue

Show
	ItemLevel 9
	CustomAlertSound "None"
	Continue

Show
	ItemLevel 10
	CustomAlertSound "None"
	Continue

Show
	ItemLevel 1
	PlayAlertSound 1 100
	Continue

Show
	ItemLevel 2
	PlayAlertSound 2
	Continue

Show
	ItemLevel 3
	PlayAlertSound ShGeneral 300
	Continue

Show
	ItemLevel 4
	PlayAlertSound ShGeneral
	Continue

Show
	ItemLevel 5
	PlayAlertSound None
	Continue

Show
	ItemLevel 6
	PlayAlertSound None
	Continue

Show
	ItemLevel 7
	CustomAlertSound ""
	Continue

Show
	ItemLevel 8
	CustomAlertSound ""
	Continue

Show
	ItemLevel 9
	CustomAlertSound "None"
	Continue

Show
	ItemLevel 10
	CustomAlertSound "None"
	Continue

Show
	PlayAlertSound ShVaal 100

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(delirium_new_colors_and_shapes)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
MinimapIcon 0 Cyan Cross
{
	Show
}

MinimapIcon 1 Grey Moon
{
	Show
}

MinimapIcon 2 Orange Raindrop
{
	Show
}

MinimapIcon 0 Pink Kite
{
	Show
}

MinimapIcon 1 Purple Pentagon
{
	Show
}

MinimapIcon 2 Cyan UpsideDownHouse
{
	Show
}

)");
			const std::string_view expected_filter =
R"(Show
	MinimapIcon 0 Cyan Cross

Show
	MinimapIcon 1 Grey Moon

Show
	MinimapIcon 2 Orange Raindrop

Show
	MinimapIcon 0 Pink Kite

Show
	MinimapIcon 1 Purple Pentagon

Show
	MinimapIcon 2 Cyan UpsideDownHouse

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(heist_new_replica_and_alternate_quality_conditions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
BaseType "Something"
Replica True
AlternateQuality True
{
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	BaseType "Something"
	Replica True
	AlternateQuality True

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(ritual_new_ranged_string_array_conditions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
HasExplicitMod ==  "of Haast" "of Tzteosh" "of Ephij"
{
	Show
}
HasExplicitMod     "of Haast" "of Tzteosh" "of Ephij"
{
	Show
}
HasExplicitMod ==2 "of Haast" "of Tzteosh" "of Ephij"
{
	Show
}
HasExplicitMod >=2 "of Haast" "of Tzteosh" "of Ephij"
{
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	HasExplicitMod == "of Haast" "of Tzteosh" "of Ephij"

Show
	HasExplicitMod "of Haast" "of Tzteosh" "of Ephij"

Show
	HasExplicitMod ==2 "of Haast" "of Tzteosh" "of Ephij"

Show
	HasExplicitMod >=2 "of Haast" "of Tzteosh" "of Ephij"

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(scourge_new_conditions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
BaseDefencePercentile >= 90
{
	Show
}
BaseArmour            > 500
{
	Show
}
BaseEvasion           > 500
{
	Show
}
BaseEnergyShield      > 200
{
	Show
}
BaseWard              >  50
{
	Show
}
Scourged               True
{
	Show
}
UberBlightedMap        True
{
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	BaseDefencePercentile >= 90

Show
	BaseArmour > 500

Show
	BaseEvasion > 500

Show
	BaseEnergyShield > 200

Show
	BaseWard > 50

Show
	Scourged True

Show
	UberBlightedMap True

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(crucible_and_affliction_new_conditions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
HasImplicitMod         True
{
	Show
}
HasCruciblePassiveTree True
{
	Show
}
TransfiguredGem        True
{
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	HasImplicitMod True

Show
	HasCruciblePassiveTree True

Show
	TransfiguredGem True

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(siege_of_the_atlas_new_conditions)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
HasSearingExarchImplicit    5
{
	Show
}
HasSearingExarchImplicit >= 4
{
	Show
}
HasEaterOfWorldsImplicit    6
{
	Show
}
HasEaterOfWorldsImplicit >  4
{
	Show
}

ArchnemesisMod "Toxic" "Hasted"
{
	Hide
}
)");
			const std::string_view expected_filter =
R"(Show
	HasSearingExarchImplicit 5

Show
	HasSearingExarchImplicit >= 4

Show
	HasEaterOfWorldsImplicit 6

Show
	HasEaterOfWorldsImplicit > 4

Hide
	ArchnemesisMod "Toxic" "Hasted"

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(shaper_voice_lines)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
PlayAlertSound ShExalted
{
	Show
}

PlayAlertSound ShDivine 250
{
	Show
}

PlayAlertSoundPositional ShRegal
{
	Show
}

PlayAlertSoundPositional ShFusing 200
{
	Show
}

SetAlertSound ShAlchemy
{
	Show
}

SetAlertSound ShVaal 150
{
	Show
}

)");
			const std::string_view expected_filter =
R"(Show
	PlayAlertSound ShExalted

Show
	PlayAlertSound ShDivine 250

Show
	PlayAlertSoundPositional ShRegal

Show
	PlayAlertSoundPositional ShFusing 200

Show
	PlayAlertSound ShAlchemy

Show
	PlayAlertSound ShVaal 150

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(custom_alert_sound)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
CustomAlertSound "pop.wav"
{
	Show
}

CustomAlertSound "pop.wav" 123
{
	Show
}

CustomAlertSoundOptional "pop.wav"
{
	Show
}

CustomAlertSoundOptional "pop.wav" 123
{
	Show
}
)");
			const std::string_view expected_filter =
R"(Show
	CustomAlertSound "pop.wav"

Show
	CustomAlertSound "pop.wav" 123

Show
	CustomAlertSoundOptional "pop.wav"

Show
	CustomAlertSoundOptional "pop.wav" 123

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(switch_drop_sound)
		{
			const std::string actual_filter = generate_filter(minimal_input() + R"(
SetFontSize 30

DropLevel >= 10
{
	SetFontSize 35

	ItemLevel < 70
	{
		EnableDropSound
		Hide
	}

	Rarity >= Rare
	ItemLevel >= 75
	{
		SetFontSize 40
		DisableDropSoundIfAlertSound
		Show
	}

	Rarity >= Rare
	ItemLevel >= 70
	{
		SetFontSize 40
		DisableDropSound
		Show
	}

	Sockets 6
	{
		PlayEffect Blue
		Show
		Continue
	}

	EnableDropSound
	Show
}

EnableDropSoundIfAlertSound
Hide
)");
			const std::string_view expected_filter =
R"(Hide
	DropLevel >= 10
	ItemLevel < 70
	SetFontSize 35
	EnableDropSound

Show
	DropLevel >= 10
	Rarity >= Rare
	ItemLevel >= 75
	SetFontSize 40
	DisableDropSoundIfAlertSound

Show
	DropLevel >= 10
	Rarity >= Rare
	ItemLevel >= 70
	SetFontSize 40
	DisableDropSound

Show
	DropLevel >= 10
	Sockets 6
	SetFontSize 35
	PlayEffect Blue
	Continue

Show
	DropLevel >= 10
	SetFontSize 35
	EnableDropSound

Hide
	SetFontSize 30
	EnableDropSoundIfAlertSound

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

		BOOST_AUTO_TEST_CASE(simple_price_queries)
		{
			using lang::market::divination_card;
			using lang::market::price_data;

			lang::market::item_price_data ipd;
			ipd.divination_cards.push_back(divination_card{price_data{0.125, false}, "Rain of Chaos", 8});
			ipd.divination_cards.push_back(divination_card{price_data{5, false}, "Humility", 9});
			ipd.divination_cards.push_back(divination_card{price_data{10, false}, "A Dab of Ink", 9});
			ipd.divination_cards.push_back(divination_card{price_data{100, false}, "Abandoned Wealth", 5});
			ipd.divination_cards.push_back(divination_card{price_data{1000, false}, "The Doctor", 8});
			const std::string actual_filter = generate_filter(minimal_input() + R"(
Class "Divination Card"
Autogen "cards"
{
	Price >= 100
	{
		Show
	}

	Price < 100
	Price >= 10
	{
		Show
	}

	Price < 10
	Price >= 5
	{
		Show
	}

	Price < 5
	{
		Hide
	}
}
)", ipd);
			const std::string_view expected_filter =
R"(Show
	Class == "Divination Card"
	BaseType == "Abandoned Wealth" "The Doctor"

Show
	Class == "Divination Card"
	BaseType == "A Dab of Ink"

Show
	Class == "Divination Card"
	BaseType == "Humility"

Hide
	Class == "Divination Card"
	BaseType == "Rain of Chaos"

)";

			BOOST_TEST(compare_strings(expected_filter, actual_filter));
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
