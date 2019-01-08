#pragma once

#include <boost/spirit/home/x3.hpp>

namespace fs::parser
{

namespace x3 = boost::spirit::x3;

// symbols that denote filter's language constants
struct booleans_ : x3::symbols<bool>
{
	booleans_()
	{
		add
			("True", true)
			("False", false)
		;
	}

};
const booleans_ booleans;


// some constants to aovid code duplication
constexpr auto keyword_boolean  = "Boolean";
constexpr auto keyword_number   = "Number";
constexpr auto keyword_level    = "Level";
constexpr auto keyword_sound_id = "SoundId";
constexpr auto keyword_volume   = "Volume";
constexpr auto keyword_color    = "Color";
constexpr auto keyword_true     = "True";
constexpr auto keyword_false    = "False";
constexpr auto assignment_operator = '=';
constexpr auto newline_character   = '\n';

}
