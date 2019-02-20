#pragma once

/*
 * some of these constants would seem to be very trivial but:
 * - code duplication still allows to make typos in strings
 * - having centralized place of language constants makes:
 *   - searching for usage of code in the IDE easier
 *   - refactoring code easier
 *   - changing filter language easier
 */
namespace fs::lang::keywords
{

constexpr auto boolean      = "Boolean";
constexpr auto integer      = "Integer";
constexpr auto level        = "Level";
constexpr auto sound_id     = "SoundId";
constexpr auto volume       = "Volume";
constexpr auto font_size    = "FontSize";
constexpr auto group        = "Group";
constexpr auto rarity_type  = "RarityType";
constexpr auto shape        = "Shape";
constexpr auto suit         = "Suit";
constexpr auto color        = "Color";
constexpr auto minimap_icon = "MinimapIcon";
constexpr auto beam_effect  = "BeamEffect";
constexpr auto string       = "String";
constexpr auto path         = "Path";
constexpr auto alert_sound  = "AlertSound";

constexpr auto r = 'R';
constexpr auto g = 'G';
constexpr auto b = 'B';
constexpr auto w = 'W';

constexpr auto show = "Show";
constexpr auto hide = "Hide";

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

constexpr const char* const const_ = "const"; // (for meme purposes only)

// ---- rules ----

constexpr auto item_level       = "ItemLevel";
constexpr auto drop_level       = "DropLevel";
constexpr auto quality          = "Quality";
constexpr auto rarity           = "Rarity";
constexpr auto class_           = "Class";
constexpr auto base_type        = "BaseType";
constexpr auto sockets          = "Sockets";
constexpr auto linked_sockets   = "LinkedSockets";
constexpr auto socket_group     = "SocketGroup";
constexpr auto height           = "Height";
constexpr auto width            = "Width";
constexpr auto has_explicit_mod = "HasExplicitMod";
constexpr auto stack_size       = "StackSize";
constexpr auto gem_level        = "GemLevel";
constexpr auto identified       = "Identified";
constexpr auto corrupted        = "Corrupted";
constexpr auto elder_item       = "ElderItem";
constexpr auto shaper_item      = "ShaperItem";
constexpr auto shaped_map       = "ShapedMap";
constexpr auto map_tier         = "MapTier";

constexpr auto set_border_color           = "SetBorderColor";
constexpr auto set_text_color             = "SetTextColor";
constexpr auto set_background_color       = "SetBackgroundColor";
constexpr auto set_font_size              = "SetFontSize";
constexpr auto set_alert_sound            = "SetAlertSound";
constexpr auto set_alert_sound_positional = "SetAlertSoundPositional";
constexpr auto disable_drop_sound         = "DisableDropSound";
constexpr auto set_minimap_icon           = "SetMinimapIcon";
constexpr auto set_beam                   = "SetBeam";

}
