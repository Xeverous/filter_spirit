#include <fs/network/poe_ninja/api_data.hpp>
#include <fs/log/logger.hpp>
#include <fs/utility/file.hpp>

#include <utility>

namespace
{

constexpr auto filename_currency = "Currency.json";
constexpr auto filename_fragment = "Fragment.json";
constexpr auto filename_delirium_orb = "DeliriumOrb.json";
constexpr auto filename_oil = "Oil.json";
constexpr auto filename_incubator = "Incubator.json";
constexpr auto filename_scarab = "Scarab.json";
constexpr auto filename_fossil = "Fossil.json";
constexpr auto filename_resonator = "Resonator.json";
constexpr auto filename_essence = "Essence.json";
constexpr auto filename_divination_card = "DivinationCard.json";
constexpr auto filename_skill_gem = "SkillGem.json";
constexpr auto filename_base_type = "BaseType.json";
constexpr auto filename_tattoo = "Tattoo.json";
constexpr auto filename_unique_map = "UniqueMap.json";
constexpr auto filename_map = "Map.json";
constexpr auto filename_unique_jewel = "UniqueJewel.json";
constexpr auto filename_unique_flask = "UniqueFlask.json";
constexpr auto filename_unique_weapon = "UniqueWeapon.json";
constexpr auto filename_unique_armour = "UniqueArmour.json";
constexpr auto filename_unique_accessory = "UniqueAccessory.json";
constexpr auto filename_vial = "Vial.json";

}

namespace fs::network::poe_ninja
{

#define FOR_ALL_MEMBERS(MACRO) \
	MACRO(currency, filename_currency) \
	MACRO(fragment, filename_fragment) \
	MACRO(delirium_orb, filename_delirium_orb) \
	MACRO(oil, filename_oil) \
	MACRO(incubator, filename_incubator) \
	MACRO(scarab, filename_scarab) \
	MACRO(fossil, filename_fossil) \
	MACRO(resonator, filename_resonator) \
	MACRO(essence, filename_essence) \
	MACRO(divination_card, filename_divination_card) \
	MACRO(skill_gem, filename_skill_gem) \
	MACRO(base_type, filename_base_type) \
	MACRO(tattoo, filename_tattoo) \
	MACRO(unique_map, filename_unique_map) \
	MACRO(map, filename_map) \
	MACRO(unique_jewel, filename_unique_jewel) \
	MACRO(unique_flask, filename_unique_flask) \
	MACRO(unique_weapon, filename_unique_weapon) \
	MACRO(unique_armour, filename_unique_armour) \
	MACRO(unique_accessory, filename_unique_accessory) \
	MACRO(vial, filename_vial)

bool api_item_price_data::save(const std::filesystem::path& directory, log::logger& logger) const
{
#define SAVE(member_var, file_name) \
	if (!utility::save_file(directory / file_name, member_var, logger)) {\
		return false; \
	}

	FOR_ALL_MEMBERS(SAVE)
#undef SAVE

	return true;
}

bool api_item_price_data::load(const std::filesystem::path& directory, log::logger& logger)
{
	std::optional<std::string> file_contents;
#define LOAD(member_var, file_name) \
	file_contents = utility::load_file(directory / file_name, logger); \
	if (file_contents) {\
		member_var = std::move(*file_contents); \
	} \
	else { \
		return false; \
	}

	FOR_ALL_MEMBERS(LOAD)
#undef LOAD

	return true;
}

#undef FOR_ALL_MEMBERS

}
