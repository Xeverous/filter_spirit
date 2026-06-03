#pragma once

#include <fs/log/logger.hpp>

#include <string>
#include <filesystem>

namespace fs::network::poe_ninja
{

// FileStem - used in API endpoint URL and as a file stem (stem = filename without extension)
// IsCurrency - (ninja-specific) whether to use exchange or stash for URL and data model
template <const char* FileStem, bool IsCurrency = false>
struct json_file
{
	std::string file_content;
};

namespace poe1
{

// string literals can not be used as NTTP directly
// however, global objects of type array of char are fine

// add "inline" to "constexpr" to avoid
// GCC warning: "[...] has a field [...] whose type has internal linkage [-Wsubobject-linkage]"

// not used by FS, only applies to Phrecia event leagues and has only legacy data model
inline constexpr const char file_stem_unique_idol[] = "UniqueIdol";

// general
inline constexpr const char file_stem_currency[] = "Currency";
inline constexpr const char file_stem_fragment[] = "Fragment";
inline constexpr const char file_stem_runegraft[] = "Runegraft";
inline constexpr const char file_stem_allflame_ember[] = "AllflameEmber";
inline constexpr const char file_stem_tattoo[] = "Tattoo";
inline constexpr const char file_stem_omen[] = "Omen";
inline constexpr const char file_stem_divination_card[] = "DivinationCard";
inline constexpr const char file_stem_artifact[] = "Artifact";
inline constexpr const char file_stem_oil[] = "Oil";
inline constexpr const char file_stem_incubator[] = "Incubator";

// equipment & gems
inline constexpr const char file_stem_unique_weapon[] = "UniqueWeapon";
inline constexpr const char file_stem_unique_armour[] = "UniqueArmour";
inline constexpr const char file_stem_unique_accessory[] = "UniqueAccessory";
inline constexpr const char file_stem_unique_flask[] = "UniqueFlask";
inline constexpr const char file_stem_unique_jewel[] = "UniqueJewel";
inline constexpr const char file_stem_forbidden_jewel[] = "ForbiddenJewel";
inline constexpr const char file_stem_unique_tincture[] = "UniqueTincture";
inline constexpr const char file_stem_unique_relic[] = "UniqueRelic";
inline constexpr const char file_stem_skill_gem[] = "SkillGem";
inline constexpr const char file_stem_cluster_jewel[] = "ClusterJewel";

// Atlas
inline constexpr const char file_stem_map[] = "Map";
inline constexpr const char file_stem_blighted_map[] = "BlightedMap";
inline constexpr const char file_stem_blight_ravaged_map[] = "BlightRavagedMap";
inline constexpr const char file_stem_unique_map[] = "UniqueMap";
inline constexpr const char file_stem_delirium_orb[] = "DeliriumOrb";
inline constexpr const char file_stem_invitation[] = "Invitation";
inline constexpr const char file_stem_scarab[] = "Scarab";
inline constexpr const char file_stem_uncursion_temple[] = "IncursionTemple";

// crafting
inline constexpr const char file_stem_base_type[] = "BaseType";
inline constexpr const char file_stem_fossil[] = "Fossil";
inline constexpr const char file_stem_resonator[] = "Resonator";
inline constexpr const char file_stem_beast[] = "Beast";
inline constexpr const char file_stem_essence[] = "Essence";
inline constexpr const char file_stem_vial[] = "Vial";

// JSON file contents returned by querying poe.ninja
// https://poe.ninja/swagger (no longer available)
struct api_item_price_data
{
	// ordinary functions

	[[nodiscard]] static int num_files();
	[[nodiscard]] bool save(const std::filesystem::path& directory, log::logger& logger) const;
	[[nodiscard]] bool load(const std::filesystem::path& directory, log::logger& logger);

	// #include api_data_adapted.hpp to obtain these

	// F must accept (const char* name, bool is_currency, std::string& content)
	template <typename F>
	void for_each_file(F f);

	// F must accept (const char* name, bool is_currency, const std::string& content)
	template <typename F>
	void for_each_file(F f) const;

	// order as on poe.ninja

	// general
	json_file<file_stem_currency, true> currency;
	json_file<file_stem_fragment, true> fragment;
	json_file<file_stem_runegraft, true> runegraft;
	json_file<file_stem_allflame_ember, true> allflame_ember;
	json_file<file_stem_tattoo, true> tattoo;
	json_file<file_stem_omen, true> omen;
	json_file<file_stem_divination_card, true> divination_card;
	json_file<file_stem_artifact, true> artifact;
	json_file<file_stem_oil, true> oil;
	json_file<file_stem_incubator> incubator;

	// equipment & gems
	json_file<file_stem_unique_weapon> unique_weapon;
	json_file<file_stem_unique_armour> unique_armour;
	json_file<file_stem_unique_accessory> unique_accessory;
	json_file<file_stem_unique_flask> unique_flask;
	json_file<file_stem_unique_jewel> unique_jewel;
	json_file<file_stem_unique_tincture> unique_tincture;
	json_file<file_stem_unique_relic> unique_relic;
	json_file<file_stem_skill_gem> skill_gem;
	json_file<file_stem_cluster_jewel> cluster_jewel;

	// Atlas
	json_file<file_stem_map> map;
	json_file<file_stem_blighted_map> blighted_map;
	json_file<file_stem_blight_ravaged_map> blight_ravaged_map;
	json_file<file_stem_unique_map> unique_map;
	json_file<file_stem_delirium_orb, true> delirium_orb;
	json_file<file_stem_invitation> invitation;
	json_file<file_stem_scarab, true> scarab;

	// crafting
	json_file<file_stem_base_type> base_type;
	json_file<file_stem_fossil, true> fossil;
	json_file<file_stem_resonator, true> resonator;
	json_file<file_stem_beast> beast;
	json_file<file_stem_essence, true> essence;
	json_file<file_stem_vial> vial;
};

} // namespace poe1

namespace poe2 {

inline constexpr const char file_stem_currency[] = "Currency";
inline constexpr const char file_stem_fragments[] = "Fragments";
inline constexpr const char file_stem_abyss[] = "Abyss";
inline constexpr const char file_stem_uncut_gems[] = "UncutGems";
inline constexpr const char file_stem_lineage_support_gems[] = "LineageSupportGems";
inline constexpr const char file_stem_essences[] = "Essences";
inline constexpr const char file_stem_ultimatum[] = "Ultimatum";
inline constexpr const char file_stem_idols[] = "Idols";
inline constexpr const char file_stem_runes[] = "Runes";
inline constexpr const char file_stem_ritual[] = "Ritual";
inline constexpr const char file_stem_expedition[] = "Expedition";
inline constexpr const char file_stem_delirium[] = "Delirium";
inline constexpr const char file_stem_breach[] = "Breach";
inline constexpr const char file_stem_verisium[] = "Verisium";

struct api_item_price_data
{
	// ordinary functions

	[[nodiscard]] static int num_files();
	[[nodiscard]] bool save(const std::filesystem::path& directory, log::logger& logger) const;
	[[nodiscard]] bool load(const std::filesystem::path& directory, log::logger& logger);

	// #include api_data_adapted.hpp to obtain these

	// F must accept (const char* name, std::string& content)
	template <typename F>
	void for_each_file(F f);

	// F must accept (const char* name, std::string& content)
	template <typename F>
	void for_each_file(F f) const;

	// order as on poe.ninja
	json_file<file_stem_currency, true> currency;
	json_file<file_stem_fragments, true> fragments; // Map Fragments + "Runic Splinter"
	json_file<file_stem_abyss, true> abyss; // Abyss currency + "Kulemak's Invitation"
	json_file<file_stem_uncut_gems, true> uncut_gems;
	json_file<file_stem_lineage_support_gems, true> lineage_support_gems;
	json_file<file_stem_essences, true> essences;
	json_file<file_stem_ultimatum, true> ultimatum; // Soul Cores
	json_file<file_stem_idols, true> idols;
	json_file<file_stem_runes, true> runes;
	json_file<file_stem_ritual, true> ritual; // Omens + "Petition Splinter"
	json_file<file_stem_expedition, true> expedition;
	json_file<file_stem_delirium, true> delirium; // Emotions + "Simulacrum Splinter"
	json_file<file_stem_breach, true> breach; // Catalysts + "Breach Splinter"
	json_file<file_stem_verisium, true> verisium;
};

} // namespace poe2

}
