#pragma once

#include <fs/log/logger.hpp>

#include <string>
#include <filesystem>

namespace fs::network::poe_ninja
{

namespace poe1
{

// string literals can not be used as NTTP directly
// however, global objects of type array of char are fine

// add "inline" to "constexpr" to avoid
// GCC warning: "[...] has a field [...] whose type has internal linkage [-Wsubobject-linkage]"

// not used by FS, only applies to Phrecia event leagues and has only legacy data model
inline constexpr const char filename_unique_idol[] = "UniqueIdol";

// general
inline constexpr const char filename_currency[] = "Currency";
inline constexpr const char filename_fragment[] = "Fragment";
inline constexpr const char filename_runegraft[] = "Runegraft";
inline constexpr const char filename_allflame_ember[] = "AllflameEmber";
inline constexpr const char filename_tattoo[] = "Tattoo";
inline constexpr const char filename_omen[] = "Omen";
inline constexpr const char filename_divination_card[] = "DivinationCard";
inline constexpr const char filename_artifact[] = "Artifact";
inline constexpr const char filename_oil[] = "Oil";
inline constexpr const char filename_incubator[] = "Incubator";

// equipment & gems
inline constexpr const char filename_unique_weapon[] = "UniqueWeapon";
inline constexpr const char filename_unique_armour[] = "UniqueArmour";
inline constexpr const char filename_unique_accessory[] = "UniqueAccessory";
inline constexpr const char filename_unique_flask[] = "UniqueFlask";
inline constexpr const char filename_unique_jewel[] = "UniqueJewel";
inline constexpr const char filename_forbidden_jewel[] = "ForbiddenJewel";
inline constexpr const char filename_unique_tincture[] = "UniqueTincture";
inline constexpr const char filename_unique_relic[] = "UniqueRelic";
inline constexpr const char filename_skill_gem[] = "SkillGem";
inline constexpr const char filename_cluster_jewel[] = "ClusterJewel";

// Atlas
inline constexpr const char filename_map[] = "Map";
inline constexpr const char filename_blighted_map[] = "BlightedMap";
inline constexpr const char filename_blight_ravaged_map[] = "BlightRavagedMap";
inline constexpr const char filename_unique_map[] = "UniqueMap";
inline constexpr const char filename_delirium_orb[] = "DeliriumOrb";
inline constexpr const char filename_invitation[] = "Invitation";
inline constexpr const char filename_scarab[] = "Scarab";
inline constexpr const char filename_uncursion_temple[] = "IncursionTemple";

// crafting
inline constexpr const char filename_base_type[] = "BaseType";
inline constexpr const char filename_fossil[] = "Fossil";
inline constexpr const char filename_resonator[] = "Resonator";
inline constexpr const char filename_beast[] = "Beast";
inline constexpr const char filename_essence[] = "Essence";
inline constexpr const char filename_vial[] = "Vial";

// name - used in API endpoint URL and as a file name
// currency - (ninja-specific) whether to use exchange or stash URL and data model
template <const char* Name, bool IsCurrency = false>
struct json_file
{
	std::string file_content;
};

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
	json_file<filename_currency, true> currency;
	json_file<filename_fragment, true> fragment;
	json_file<filename_runegraft, true> runegraft;
	json_file<filename_allflame_ember, true> allflame_ember;
	json_file<filename_tattoo, true> tattoo;
	json_file<filename_omen, true> omen;
	json_file<filename_divination_card, true> divination_card;
	json_file<filename_artifact, true> artifact;
	json_file<filename_oil, true> oil;
	json_file<filename_incubator> incubator;

	// equipment & gems
	json_file<filename_unique_weapon> unique_weapon;
	json_file<filename_unique_armour> unique_armour;
	json_file<filename_unique_accessory> unique_accessory;
	json_file<filename_unique_flask> unique_flask;
	json_file<filename_unique_jewel> unique_jewel;
	json_file<filename_unique_tincture> unique_tincture;
	json_file<filename_unique_relic> unique_relic;
	json_file<filename_skill_gem> skill_gem;
	json_file<filename_cluster_jewel> cluster_jewel;

	// Atlas
	json_file<filename_map> map;
	json_file<filename_blighted_map> blighted_map;
	json_file<filename_blight_ravaged_map> blight_ravaged_map;
	json_file<filename_unique_map> unique_map;
	json_file<filename_delirium_orb, true> delirium_orb;
	json_file<filename_invitation> invitation;
	json_file<filename_scarab, true> scarab;

	// crafting
	json_file<filename_base_type> base_type;
	json_file<filename_fossil, true> fossil;
	json_file<filename_resonator, true> resonator;
	json_file<filename_beast> beast;
	json_file<filename_essence, true> essence;
	json_file<filename_vial> vial;
};

} // namespace poe1

namespace poe2 {

inline constexpr const char filename_currency[] = "Currency";
inline constexpr const char filename_fragments[] = "Fragments";
inline constexpr const char filename_abyss[] = "Abyss";
inline constexpr const char filename_uncut_gems[] = "UncutGems";
inline constexpr const char filename_lineage_support_gems[] = "LineageSupportGems";
inline constexpr const char filename_essences[] = "Essences";
inline constexpr const char filename_ultimatum[] = "Ultimatum";
inline constexpr const char filename_idols[] = "Idols";
inline constexpr const char filename_runes[] = "Runes";
inline constexpr const char filename_ritual[] = "Ritual";
inline constexpr const char filename_expedition[] = "Expedition";
inline constexpr const char filename_delirium[] = "Delirium";
inline constexpr const char filename_breach[] = "Breach";

template <const char* Name, bool IsCurrency = false>
struct json_file
{
	std::string file_content;
};

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
	json_file<filename_currency> currency;
	json_file<filename_fragments> fragments; // Map Fragments + "Runic Splinter"
	json_file<filename_abyss> abyss; // Abyss currency + "Kulemak's Invitation"
	json_file<filename_uncut_gems> uncut_gems;
	json_file<filename_lineage_support_gems> lineage_support_gems;
	json_file<filename_essences> essences;
	json_file<filename_ultimatum> ultimatum; // Soul Cores
	json_file<filename_idols> idols;
	json_file<filename_runes> runes;
	json_file<filename_ritual> ritual; // Omens + "Petition Splinter"
	json_file<filename_expedition> expedition;
	json_file<filename_delirium> delirium; // Emotions + "Simulacrum Splinter"
	json_file<filename_breach> breach; // Catalysts + "Breach Splinter"
};

} // namespace poe2

}
