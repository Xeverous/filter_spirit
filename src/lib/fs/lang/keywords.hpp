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
	constexpr auto area_level       = "AreaLevel";
	constexpr auto corrupted_mods   = "CorruptedMods";

	constexpr auto r = 'R';
	constexpr auto g = 'G';
	constexpr auto b = 'B';
	constexpr auto w = 'W';
	constexpr auto a = 'A';
	constexpr auto d = 'D';

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
	constexpr auto none = "None";
} // namespace rf

// keywords used in spirit filters
namespace sf
{
	// ---- condition extensions ----

	// autogeneration feature
	constexpr auto price = "Price";
	constexpr auto autogen = "Autogen";

	// some item types are commented - there is no point in querying for them
	// and/or sites like poe.watch and poe.ninja do not report their prices
	constexpr auto cards           = "cards";           // 2.0 - The Awakening (Warbands + Tempest)
	constexpr auto enchants_helmet = "enchants_helmet"; // 2.2 - Perandus league (also Ascendancy)
	constexpr auto prophecies      = "prophecies";      // 2.3 - Prophecy league
	constexpr auto essences        = "essences";        // 2.4 - Essence league (also Atlas of Worlds)
	constexpr auto leaguestones    = "leaguestones";    // 2.6 - Legacy league
	// constexpr auto pieces       = "pieces";          // 3.0 - Harbinger league (also Fall of Oriath)
	// constexpr auto nets         = "nets";            // 3.2 - Bestiary league
	// constexpr auto vials        = "vials";           // 3.3 - Incursion league
	constexpr auto fossils         = "fossils";         // 3.4 - Delve league
	constexpr auto resonators      = "resonators";      // as above
	constexpr auto scarabs         = "scarabs";         // 3.5 - Betrayal league
	constexpr auto incubators      = "incubators";      // 3.7 - Legion league
	constexpr auto oils            = "oils";            // 3.8 - Blight league
	constexpr auto catalysts       = "catalysts";       // 3.9 - Metamorph league


	constexpr auto uniques_eq_unambiguous    = "uniques_eq_unambiguous";
	constexpr auto uniques_eq_ambiguous      = "uniques_eq_ambiguous";

	constexpr auto uniques_flasks_unambiguous = "uniques_flasks_unambiguous";
	constexpr auto uniques_flasks_ambiguous   = "uniques_flasks_ambiguous";

	constexpr auto uniques_jewels_unambiguous = "uniques_jewels_unambiguous";
	constexpr auto uniques_jewels_ambiguous   = "uniques_jewels_ambiguous";

	constexpr auto uniques_maps_unambiguous   = "uniques_maps_unambiguous";
	constexpr auto uniques_maps_ambiguous     = "uniques_maps_ambiguous";

	// ---- action extensions ----
	constexpr auto set_alert_sound = "SetAlertSound";
} // namespace sf

}
