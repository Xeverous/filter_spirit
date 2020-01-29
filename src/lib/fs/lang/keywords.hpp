#pragma once

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

// keywords used in real filters and potentially used in spirit filter templates
namespace rf
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
	constexpr auto prophecy         = "Prophecy";
	constexpr auto has_influence    = "HasInfluence";
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
	constexpr auto elder_map        = "ElderMap";
	constexpr auto blighted_map     = "BlightedMap";
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
	constexpr auto diamond  = "Diamond";
	constexpr auto hexagon  = "Hexagon";
	constexpr auto square   = "Square";
	constexpr auto star     = "Star";
	constexpr auto triangle = "Triangle";

	constexpr auto shaper      = "Shaper";
	constexpr auto elder       = "Elder";
	constexpr auto crusader    = "Crusader";
	constexpr auto redeemer    = "Redeemer";
	constexpr auto hunter      = "Hunter";
	constexpr auto warlord     = "Warlord";

	constexpr auto temp = "Temp";
} // namespace rf

// keywords used in spirit filters
namespace sf
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

	constexpr auto set_alert_sound            = "SetAlertSound";
	constexpr auto play_default_drop_sound    = "PlayDefaultDropSound";
	constexpr auto set_minimap_icon           = "SetMinimapIcon";
	constexpr auto set_beam                   = "SetBeam";
} // namespace sf

}
