#pragma once

// stuff for the real item filter imported by Path of Exile client
namespace fs::lang::generation
{

// visibility
constexpr auto show = "Show";
constexpr auto hide = "Hide";

// conditions
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
constexpr auto has_enchantment  = "HasEnchantment";
constexpr auto stack_size       = "StackSize";
constexpr auto gem_level        = "GemLevel";
constexpr auto identified       = "Identified";
constexpr auto corrupted        = "Corrupted";
constexpr auto elder_item       = "ElderItem";
constexpr auto shaper_item      = "ShaperItem";
constexpr auto fractured_item   = "FracturedItem";
constexpr auto synthesised_item = "SynthesisedItem";
constexpr auto any_enchantment  = "AnyEnchantment";
constexpr auto shaped_map       = "ShapedMap";
constexpr auto map_tier         = "MapTier";

constexpr auto r = 'R';
constexpr auto g = 'G';
constexpr auto b = 'B';
constexpr auto w = 'W';

constexpr auto normal = "Normal";
constexpr auto magic  = "Magic";
constexpr auto rare   = "Rare";
constexpr auto unique = "Unique";

constexpr auto true_   = "True";
constexpr auto false_  = "False";

// actions
constexpr auto set_border_color            = "SetBorderColor";
constexpr auto set_text_color              = "SetTextColor";
constexpr auto set_background_color        = "SetBackgroundColor";
constexpr auto set_font_size               = "SetFontSize";
constexpr auto play_alert_sound            = "PlayAlertSound";
constexpr auto play_alert_sound_positional = "PlayAlertSoundPositional";
constexpr auto disable_drop_sound          = "DisableDropSound";
constexpr auto custom_alert_sound          = "CustomAlertSound";
constexpr auto minimap_icon                = "MinimapIcon";
constexpr auto play_effect                 = "PlayEffect";

constexpr auto red    = "Red";
constexpr auto green  = "Green";
constexpr auto blue   = "Blue";
constexpr auto white  = "White";
constexpr auto brown  = "Brown";
constexpr auto yellow = "Yellow";

constexpr auto circle   = "Circle";
constexpr auto diamond  = "Diamon";
constexpr auto hexagon  = "Hexagon";
constexpr auto square   = "Square";
constexpr auto star     = "Star";
constexpr auto triangle = "Triangle";

constexpr auto temp = "Temp";

}
