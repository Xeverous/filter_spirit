#pragma once
#include "fs/lang/generation.hpp"

/*
 * some of these constants would seem to be very trivial but:
 * - code duplication still allows to make typos in strings
 * - having centralized place of language constants makes:
 *   - searching for usage of code in the IDE easier
 *   - refactoring code easier
 *   - changing filter language easier
 *   - finding differences between Filter Spirit and the real
 *     filter accepted by game client easier - some keywords are
 *     different for more consistent filter templates
 */
namespace fs::lang::keywords
{

constexpr auto level        = "Level";
constexpr auto font_size    = "FontSize";
constexpr auto sound_id     = "SoundId";
constexpr auto volume       = "Volume";
constexpr auto group        = "Group";
constexpr auto minimap_icon = "MinimapIcon";
constexpr auto beam_effect  = "Beam";
constexpr auto path         = "Path";
constexpr auto alert_sound  = "AlertSound";

constexpr auto show = generation::show;
constexpr auto hide = generation::hide;

// ---- whitespace ----

// (obviously no keywords for whitespace)

// ---- fundamental tokens ----

// (tokens only, no keywords here)

// ---- version requirement ----

constexpr auto version = "version";

// ---- config ----

constexpr auto config = "config";
constexpr auto yes = "yes";
constexpr auto no = "no";

// ---- literal types ----

constexpr auto true_       = "true";
constexpr auto false_      = "false";

constexpr auto normal      = "normal";
constexpr auto magic       = "magic";
constexpr auto rare        = "rare";
constexpr auto unique      = "unique";

constexpr auto circle      = "circle";
constexpr auto diamond     = "diamond";
constexpr auto hexagon     = "hexagon";
constexpr auto square      = "square";
constexpr auto star        = "star";
constexpr auto triangle    = "triangle";

constexpr auto red         = "red";
constexpr auto green       = "green";
constexpr auto blue        = "blue";
constexpr auto white       = "white";
constexpr auto brown       = "brown";
constexpr auto yellow      = "yellow";

// ---- expressions ----

// (nothing)

// ---- definitions ----

constexpr const char* const const_ = "const"; // const

// ---- rules ----

constexpr auto item_level       = generation::item_level;
constexpr auto drop_level       = generation::drop_level;
constexpr auto quality          = generation::quality;
constexpr auto rarity           = generation::rarity;
constexpr auto class_           = generation::class_;
constexpr auto base_type        = generation::base_type;
constexpr auto sockets          = generation::sockets;
constexpr auto linked_sockets   = generation::linked_sockets;
constexpr auto socket_group     = generation::socket_group;
constexpr auto height           = generation::height;
constexpr auto width            = generation::width;
constexpr auto has_explicit_mod = generation::has_explicit_mod;
constexpr auto stack_size       = generation::stack_size;
constexpr auto gem_level        = generation::gem_level;
constexpr auto identified       = generation::identified;
constexpr auto corrupted        = generation::corrupted;
constexpr auto elder_item       = generation::elder_item;
constexpr auto shaper_item      = generation::shaper_item;
constexpr auto fractured_item   = generation::fractured_item;
constexpr auto synthesised_item = generation::synthesised_item;
constexpr auto any_enchantment  = generation::any_enchantment;
constexpr auto shaped_map       = generation::shaped_map;
constexpr auto map_tier         = generation::map_tier;

constexpr auto set_border_color           = generation::set_border_color;
constexpr auto set_text_color             = generation::set_text_color;
constexpr auto set_background_color       = generation::set_background_color;
constexpr auto set_font_size              = generation::set_font_size;
constexpr auto set_alert_sound            = "SetAlertSound";
constexpr auto play_default_drop_sound    = "PlayDefaultDropSound";
constexpr auto set_minimap_icon           = "SetMinimapIcon";
constexpr auto set_beam                   = "SetBeam";

}
