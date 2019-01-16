#pragma once

/*
 * some of these constants would seem to be very trivial but:
 * - code duplication still allows to make typos in strings
 * - having centralized place of language constants makes:
 *   - searching for usage of code in the IDE easier
 *   - refactoring code easier
 *   - changing filter language easier
 */
namespace fs::lang::constants
{

	namespace keywords
	{
		constexpr auto true_    = "True";
		constexpr auto false_   = "False";

		constexpr auto boolean  = "Boolean";
		constexpr auto number   = "Number";
		constexpr auto level    = "Level";
		constexpr auto sound_id = "SoundId";
		constexpr auto volume   = "Volume";
		constexpr auto rarity   = "Rarity";
		constexpr auto shape    = "Shape";
		constexpr auto suit     = "Suit";
		constexpr auto color    = "Color";
		constexpr auto group    = "Group";
		constexpr auto string   = "String";

		constexpr auto normal   = "Normal";
		constexpr auto magic    = "Magic";
		constexpr auto rare     = "Rare";
		constexpr auto unique   = "Unique";

		constexpr auto circle   = "Circle";
		constexpr auto diamond  = "Diamond";
		constexpr auto hexagon  = "Hexagon";
		constexpr auto square   = "Square";
		constexpr auto star     = "Star";
		constexpr auto triangle = "Triangle";

		constexpr auto red      = "Red";
		constexpr auto green    = "Green";
		constexpr auto blue     = "Blue";
		constexpr auto white    = "White";
		constexpr auto brown    = "Brown";
		constexpr auto yellow   = "Yellow";

		constexpr auto r   = 'R';
		constexpr auto g   = 'G';
		constexpr auto b   = 'B';
		constexpr auto w   = 'W';
	}

	constexpr auto default_opacity = 255; // TODO: this is not correct, find the real value

}
