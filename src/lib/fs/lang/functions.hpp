#pragma once
#include <fs/lang/keywords.hpp>

namespace fs::lang::functions
{

// names of built-in functions
constexpr auto rgb = "RGB";
constexpr auto level        = keywords::sf::level;
// constexpr auto quality      = keywords::quality;
constexpr auto font_size    = keywords::sf::font_size;
constexpr auto sound_id     = keywords::sf::sound_id;
constexpr auto volume       = keywords::sf::volume;
constexpr auto group        = keywords::sf::group;
constexpr auto minimap_icon = keywords::sf::minimap_icon;
constexpr auto beam_effect  = keywords::sf::beam_effect;
constexpr auto path         = keywords::sf::path;
constexpr auto alert_sound  = keywords::sf::alert_sound;

}
