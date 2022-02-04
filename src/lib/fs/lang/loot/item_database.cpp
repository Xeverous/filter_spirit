#include <fs/lang/loot/item_database.hpp>
#include <fs/utility/dump_json.hpp>

#include <nlohmann/json.hpp>

#include <string_view>
#include <optional>
#include <algorithm>
#include <exception>
#include <stdexcept>

namespace {

using namespace fs;
using namespace fs::lang;

using string_t = nlohmann::json::string_t;

/*
 * walk the path in metadata: dir/dir/.../dir/name
 */
class metadata_path_walker
{
public:
	metadata_path_walker(std::string_view path)
	: _path(path) {}

	// return next *dir* name, if no more dirs, return empty string view
	std::string_view next_dir()
	{
		const auto pos = _path.find_first_of('/');
		if (pos == std::string_view::npos)
			return {};

		std::string_view result(_path.data(), pos);
		_path.remove_prefix(pos + 1); // also remove '/'
		return result;
	}

	// return filename if no more dirs are present, otherwise return empty string view
	std::string_view filename() const
	{
		const auto pos = _path.find_first_of('/');
		if (pos == std::string_view::npos)
			return _path;
		else
			return {};
	}

private:
	std::string_view _path;
};

[[noreturn]] void throw_path_error(std::string_view dir)
{
	throw std::runtime_error("unknown item at \"" + std::string(dir) + "\" in metadata ID");
}

[[noreturn]] void throw_field_error(std::string_view field, std::string_view value)
{
	throw std::runtime_error("item has unrecognized value in field \"" +  std::string(field) + "\": \"" + std::string(value) + "\"");
}

bool json_array_contains_string(const nlohmann::json& array, std::string_view string)
{
	// use any_of because nlohmann library has no .contains() for array type
	return std::any_of(array.begin(), array.end(),
		[&](const nlohmann::json& element) { return element.get_ref<const string_t&>() == string; });
}

bool tags_contains_tag(const nlohmann::json& metadata_item_tags, std::string_view tag)
{
	return json_array_contains_string(metadata_item_tags, tag);
}

bool implicits_contains_implicit(const nlohmann::json& metadata_item_implicits, std::string_view implicit)
{
	return json_array_contains_string(metadata_item_implicits, implicit);
}

bool has_has_one_socket_implicit(const nlohmann::json& metadata_item_implicits)
{
	// there are various similar implicits: TalismanHasOneSocket_, QuiverHasOneSocket, RingHasOneSocket
	return std::any_of(metadata_item_implicits.begin(), metadata_item_implicits.end(),
		[&](const nlohmann::json& imp) { return utility::contains(imp.get_ref<const string_t&>(), "HasOneSocket"); });
}

int get_max_gem_level(std::string_view gem_name, const nlohmann::json& item_tags)
{
	/*
	 * Various gems can not reach level 20; they are capped at some lower level.
	 * Unfortunately metadata contains no information what is the cap, only that is exists.
	 * Make some manual checks and throw if the gem has a level cap but was not recognized.
	 */
	if (gem_name == "Empower Support" || gem_name == "Enlighten Support" || gem_name == "Enhance Support")
		return 3;
	if (gem_name == "Portal" || gem_name == "Detonate Mines" || gem_name == "Vaal Breach")
		return 1;
	if (gem_name == "Blood and Sand" /* || gem_name == "Flesh and Stone" - this one can reach 20 */)
		return 6;
	if (utility::contains(gem_name, "Awakened"))
		return 5;

	if (tags_contains_tag(item_tags, "level_capped_gem"))
		throw std::runtime_error("unrecognized level-capped gem");

	return 20;
}

[[nodiscard]] std::optional<loot::elementary_item>
parse_elementary_item(
	std::string_view metadata_path,
	const nlohmann::json& metadata_entry)
{
	if (const auto& release_state = metadata_entry.at("release_state").get_ref<const string_t&>();
		release_state == "unique_only" || release_state == "unreleased" || release_state == "legacy")
	{
		// skip unique-only, legacy and unreleased items - they do not drop ordinarily
		return std::nullopt;
	}
	else if (release_state != "released") {
		throw_field_error("release_state", release_state);
	}

	loot::elementary_item result;
	result.metadata_path = metadata_path;
	result.name = metadata_entry.at("name").get<string_t>();
	result.drop_level = metadata_entry.at("drop_level").get<int>();
	result.width = metadata_entry.at("inventory_width").get<int>();
	result.height = metadata_entry.at("inventory_height").get<int>();
	return result;
}

// metadata contains no information on item sockets, therefore this function requires max_sockets argument
// we only check for has_one_socket implicit which appears on some bases - then override sockets to 1
[[nodiscard]] std::optional<loot::equippable_item>
parse_equippable_item(
	std::string_view metadata_path,
	const nlohmann::json& metadata_entry,
	int max_sockets)
{
	if (const auto& domain = metadata_entry.at("domain").get_ref<const string_t&>(); domain == "undefined") {
		return std::nullopt;
	}
	else if (domain != "item") {
		throw_field_error("domain", domain);
	}

	std::optional<loot::elementary_item> elem_item = parse_elementary_item(metadata_path, metadata_entry);
	if (!elem_item)
		return std::nullopt;

	loot::equippable_item result;
	static_cast<loot::elementary_item&>(result) = std::move(*elem_item);
	result.is_atlas_base_type = tags_contains_tag(metadata_entry.at("tags"), "atlas_base_type");

	if (has_has_one_socket_implicit(metadata_entry.at("implicits")))
		result.max_sockets = 1;
	else
		result.max_sockets = max_sockets;

	return result;
}

[[nodiscard]] std::optional<loot::currency_item>
parse_currency_item(
	std::string_view metadata_path,
	const nlohmann::json& metadata_entry)
{
	std::optional<loot::elementary_item> elem_item = parse_elementary_item(metadata_path, metadata_entry);
	if (!elem_item)
		return std::nullopt;

	loot::currency_item result;
	static_cast<loot::elementary_item&>(result) = std::move(*elem_item);
	result.max_stack_size = metadata_entry.at("properties").at("stack_size").get<int>();
	return result;
}

[[nodiscard]] std::optional<loot::gem>
parse_gem(
	std::string_view metadata_path,
	const nlohmann::json& metadata_entry)
{
	std::optional<loot::elementary_item> elem_item = parse_elementary_item(metadata_path, metadata_entry);
	if (!elem_item)
		return std::nullopt;

	loot::gem result;
	static_cast<loot::elementary_item&>(result) = std::move(*elem_item);
	result.max_level = get_max_gem_level(result.name, metadata_entry.at("tags"));
	return result;
}

[[nodiscard]] std::optional<loot::unique_piece>
parse_unique_piece(
	std::string_view metadata_path,
	const nlohmann::json& metadata_entry)
{
	std::optional<loot::elementary_item> elem_item = parse_elementary_item(metadata_path, metadata_entry);
	if (!elem_item)
		return std::nullopt;

	loot::unique_piece result;
	static_cast<loot::elementary_item&>(result) = std::move(*elem_item);

	if (utility::ends_with(metadata_path, "Belt1_1"))
		result.piece_name = "First Piece of Time";
	else if (utility::ends_with(metadata_path, "Belt1_2"))
		result.piece_name = "Second Piece of Time";
	else if (utility::ends_with(metadata_path, "Helmet1_1"))
		result.piece_name = "First Piece of Storms";
	else if (utility::ends_with(metadata_path, "Helmet1_2"))
		result.piece_name = "Second Piece of Storms";
	else if (utility::ends_with(metadata_path, "Helmet1_3"))
		result.piece_name = "Third Piece of Storms";
	else if (utility::ends_with(metadata_path, "Quiver1_1"))
		result.piece_name = "First Piece of Directions";
	else if (utility::ends_with(metadata_path, "Quiver1_2"))
		result.piece_name = "Second Piece of Directions";
	else if (utility::ends_with(metadata_path, "Quiver1_3"))
		result.piece_name = "Third Piece of Directions";
	else if (utility::ends_with(metadata_path, "Shield1_1"))
		result.piece_name = "First Piece of Focus";
	else if (utility::ends_with(metadata_path, "Shield1_2"))
		result.piece_name = "Second Piece of Focus";
	else if (utility::ends_with(metadata_path, "Shield1_3"))
		result.piece_name = "Third Piece of Focus";
	else if (utility::ends_with(metadata_path, "Shield1_4"))
		result.piece_name = "Fourth Piece of Focus";
	else if (utility::ends_with(metadata_path, "Staff1_1"))
		result.piece_name = "First Piece of Brutality";
	else if (utility::ends_with(metadata_path, "Staff1_2"))
		result.piece_name = "Second Piece of Brutality";
	else if (utility::ends_with(metadata_path, "Staff1_3"))
		result.piece_name = "Third Piece of Brutality";
	else if (utility::ends_with(metadata_path, "Sword1_1"))
		result.piece_name = "First Piece of the Arcane";
	else if (utility::ends_with(metadata_path, "Sword1_2"))
		result.piece_name = "Second Piece of the Arcane";
	else if (utility::ends_with(metadata_path, "Sword1_3"))
		result.piece_name = "Third Piece of the Arcane";
	else
		result.piece_name = "[UNKNOWN PIECE NAME]";

	return result;
}

[[nodiscard]] int
get_resonator_socket_count(std::string_view resonator_name)
{
	if (utility::contains(resonator_name, "Primitive"))
		return 1;
	else if (utility::contains(resonator_name, "Potent"))
		return 2;
	else if (utility::contains(resonator_name, "Powerful"))
		return 3;
	else if (utility::contains(resonator_name, "Prime"))
		return 4;
	else
		throw std::runtime_error("unrecognized resonator name");
}

[[nodiscard]] std::optional<loot::resonator>
parse_resonator(
	std::string_view metadata_path,
	const nlohmann::json& metadata_entry)
{
	std::optional<loot::currency_item> curr_item = parse_currency_item(metadata_path, metadata_entry);
	if (!curr_item)
		return std::nullopt;

	loot::resonator result;
	static_cast<loot::currency_item&>(result) = std::move(*curr_item);
	result.delve_sockets = get_resonator_socket_count(result.name);
	return result;
}

[[nodiscard]] std::optional<loot::map>
parse_map(
	std::string_view metadata_path,
	const nlohmann::json& metadata_entry)
{
	std::optional<loot::elementary_item> elem_item = parse_elementary_item(metadata_path, metadata_entry);
	if (!elem_item)
		return std::nullopt;

	if (implicits_contains_implicit(metadata_entry.at("implicits"), "MapShaperInfluence"))
		return std::nullopt; // skip guardian maps

	loot::map result;
	static_cast<loot::elementary_item&>(result) = std::move(*elem_item);
	// result.map_tier = get_map_tier(result.drop_level);
	return result;
}

void expect_unreleased_item(const nlohmann::json& metadata_entry)
{
	if (metadata_entry.at("release_state").get_ref<const string_t&>() != "unreleased")
		throw std::runtime_error("expected item to be unreleased");
}

} // namespace

namespace fs::lang::loot {

bool item_database::parse(std::string_view items_metadata_json, log::logger& logger)
{
	const auto json = nlohmann::json::parse(items_metadata_json);

	if (!json.is_object()) {
		logger.error() << "Items metadata JSON expected to be an object but is not!\n";
		return false;
	}

	for (auto [path, item_entry] : json.items()) {
		try {
			if (!item_entry.is_object())
				throw std::runtime_error("item entry is not a JSON object");

			if (path == "Metadata/items/Weapons/OneHandWeapons/OneHandSwords/OneHandSwordC")
				continue; // skip this item, it is not needed and has weird lowercase path

			metadata_path_walker walker(path);

			auto dir = walker.next_dir();
			if (dir != "Metadata")
				throw_path_error(dir);

			dir = walker.next_dir();
			if (dir != "Items")
				throw_path_error(dir);

			dir = walker.next_dir();
			if (dir == "Amulet" || dir == "Amulets") {
				std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 0);
				if (!eq_item)
					continue;

				dir = walker.next_dir();
				if (dir == "Talismans") {
					equipment.talismans.push_back(std::move(*eq_item));
				}
				else if (dir.empty()) {
					equipment.amulets.push_back(std::move(*eq_item));
				}
				else {
					throw_path_error(dir);
				}
			}
			else if (dir == "Rings") {
				std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 0);
				if (eq_item)
					equipment.rings.push_back(std::move(*eq_item));
			}
			else if (dir == "Belts") {
				std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 0);
				if (!eq_item)
					continue;

				if (implicits_contains_implicit(item_entry.at("implicits"), "AbyssJewelSocketImplicit")) {
					if ((*eq_item).name != "Stygian Vise")
						throw std::runtime_error("unknown item with an abyss socket implicit");

					equipment.stygian_vise = std::move(*eq_item);
				}
				else {
					equipment.belts.push_back(std::move(*eq_item));
				}
			}
			else if (dir == "Armours") {
				dir = walker.next_dir();

				if (dir == "BodyArmours") {
					std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 6);
					if (eq_item)
						equipment.body_armours.push_back(std::move(*eq_item));
				}
				else if (dir == "Boots") {
					std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 4);
					if (eq_item)
						equipment.boots.push_back(std::move(*eq_item));
				}
				else if (dir == "Gloves") {
					std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 4);
					if (eq_item)
						equipment.gloves.push_back(std::move(*eq_item));
				}
				else if (dir == "Helmets") {
					std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 4);
					if (eq_item)
						equipment.helmets.push_back(std::move(*eq_item));
				}
				else if (dir == "Shields") {
					std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 3);
					if (eq_item)
						equipment.shields.push_back(std::move(*eq_item));
				}
			}
			else if (dir == "Quivers") {
				std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 0);
				if (eq_item)
					equipment.quivers.push_back(std::move(*eq_item));
			}
			else if (dir == "Weapons") {
				std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 3);
				if (!eq_item)
					continue;

				dir = walker.next_dir();
				if (dir == "OneHandWeapons") {
					dir = walker.next_dir();
					if (dir == "Claws") {
						equipment.claws.push_back(std::move(*eq_item));
					}
					else if (dir == "Daggers") {
						const auto& item_class_name = item_entry.at("item_class").get_ref<const string_t&>();
						if (item_class_name == "Rune Dagger")
							equipment.rune_daggers.push_back(std::move(*eq_item));
						else
							equipment.daggers.push_back(std::move(*eq_item));
					}
					else if (dir == "OneHandAxes") {
						equipment.axes_1h.push_back(std::move(*eq_item));
					}
					else if (dir == "OneHandMaces") {
						equipment.maces_1h.push_back(std::move(*eq_item));
					}
					else if (dir == "OneHandSwords") {
						equipment.swords_1h.push_back(std::move(*eq_item));
					}
					else if (dir == "OneHandThrustingSwords") {
						equipment.thrusting_swords.push_back(std::move(*eq_item));
					}
					else if (dir == "Wands") {
						equipment.wands.push_back(std::move(*eq_item));
					}
					else {
						throw_path_error(dir);
					}
				}
				else if (dir == "TwoHandWeapon") {
					dir = walker.next_dir();
					if (dir != "FishingRods")
						throw_path_error(dir);

					std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 4);
					if (!eq_item)
						continue;

					equipment.fishing_rods.push_back(std::move(*eq_item));
				}
				else if (dir == "TwoHandWeapons") {
					std::optional<equippable_item> eq_item = parse_equippable_item(path, item_entry, 6);
					if (!eq_item)
						continue;

					dir = walker.next_dir();
					if (dir == "Bows") {
						equipment.bows.push_back(std::move(*eq_item));
					}
					else if (dir == "Staves") {
						if ((*eq_item).name == "Gnarled Branch")
							(*eq_item).max_sockets = 3;

						const auto& item_class_name = item_entry.at("item_class").get_ref<const string_t&>();
						if (item_class_name == "Warstaff")
							equipment.warstaves.push_back(std::move(*eq_item));
						else
							equipment.staves.push_back(std::move(*eq_item));
					}
					else if (dir == "TwoHandAxes") {
						equipment.axes_2h.push_back(std::move(*eq_item));
					}
					else if (dir == "TwoHandMaces") {
						equipment.maces_2h.push_back(std::move(*eq_item));
					}
					else if (dir == "TwoHandSwords") {
						equipment.swords_2h.push_back(std::move(*eq_item));
					}
					else {
						throw_path_error(dir);
					}
				}
				else {
					throw_path_error(dir);
				}
			}
			else if (dir == "AtlasExiles") {
				std::optional<currency_item> curr_item = parse_currency_item(path, item_entry);
				if (curr_item)
					currency.conqueror_orbs.push_back(std::move(*curr_item));
			}
			else if (dir == "AtlasUpgrades" /* skip Watchstones */
				|| dir == "Hideout" /* skip hideout stuff */
				|| dir == "HideoutInteractables"
				|| dir == "HideoutNPCs"
				|| dir == "ItemEffects" /* skip MTX */
				|| utility::starts_with(dir, "Microtransaction")
				|| dir == "Pets"
				|| dir == "RaceRewards"
				|| dir == "Masters" /* something weird, skip it */
				|| dir == "PantheonSouls" /* skip Pantheon Souls, they can not be dropped */)
			{
				continue;
			}
			else if (dir == "Classic") {
				expect_unreleased_item(item_entry);
				continue;
			}
			else if (dir == "Currency") {
				dir = walker.next_dir();
				if (dir == "Bestiary") {
					std::optional<currency_item> curr_item = parse_currency_item(path, item_entry);
					if (curr_item)
						currency.bestiary_nets.push_back(std::move(*curr_item));
					continue;
				}
				else if (!dir.empty()) {
					throw_path_error(dir);
				}

				const auto filename = walker.filename();
				if (utility::contains(filename, "CurrencyIncubation")) {
					// Incubators are not currency!
					std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
					if (elem_item)
						incubators.push_back(std::move(*elem_item));
					continue;
				}

				std::optional<currency_item> curr_item = parse_currency_item(path, item_entry);
				if (!curr_item)
					continue;

				if (utility::starts_with(filename, "CurrencyHarbingerBlessing")) {
					currency.harbinger_scrolls.push_back(std::move(*curr_item));
					continue;
				}
				else if (utility::starts_with(filename, "CurrencyIncursionVial")) {
					currency.incursion_vials.push_back(std::move(*curr_item));
					continue;
				}
				else if (utility::starts_with(filename, "CurrencyJewelleryQuality")) {
					currency.catalysts.push_back(std::move(*curr_item));
					continue;
				}
				else if (utility::starts_with(filename, "Mushrune")) {
					currency.oils.push_back(std::move(*curr_item));
					continue;
				}
				else if (utility::starts_with(filename, "CurrencyAfflictionOrb")) {
					currency.delirium_orbs.push_back(std::move(*curr_item));
					continue;
				}
				else if (utility::starts_with(filename, "CurrencyDelveCrafting")) {
					currency.fossils.push_back(std::move(*curr_item));
					continue;
				}
				else if (utility::starts_with(filename, "CurrencyEssence")) {
					currency.essences.push_back(std::move(*curr_item));
					continue;
				}
				else if (
					filename == "CurrencyLabyrinthEnchant" ||
					filename == "CurrencyRespecShapersOrb" ||
					utility::contains(filename, "CurrencySealMap"))
				{
					continue; // not interested in: clickable lab enchant, Unshaping Orbs / Seals (legacy Atlas)
				}

				const auto& curr_item_name = (*curr_item).name;

				if (curr_item_name == "Remnant of Corruption") {
					currency.remnant_of_corruption = std::move(*curr_item);
				}
				else if (curr_item_name == "Bestiary Orb") {
					currency.bestiary_orb = std::move(*curr_item);
				}
				else if (curr_item_name == "Albino Rhoa Feather") {
					currency.albino_rhoa_feather = std::move(*curr_item);
				}
				else if (curr_item_name == "Simulacrum Splinter") {
					currency.simulacrum_splinter = std::move(*curr_item);
				}
				else if (curr_item_name == "Imprinted Bestiary Orb" || curr_item_name == "Prophecy") {
					continue; // this is something weird that we don't want
				}
				else {
					const auto& item_tags = item_entry.at("tags");
					if (tags_contains_tag(item_tags, "breach_blessing"))
						currency.breach_blessings.push_back(std::move(*curr_item));
					else if (tags_contains_tag(item_tags, "breachstone_splinter"))
						currency.breach_splinters.push_back(std::move(*curr_item));
					else if (tags_contains_tag(item_tags, "legion_splinter"))
						currency.legion_splinters.push_back(std::move(*curr_item));
					else if (tags_contains_tag(item_tags, "currency_shard"))
						currency.generic_shards.push_back(std::move(*curr_item));
					else
						currency.generic.push_back(std::move(*curr_item));

					// implementation note - Infused Engineer's Orb - falls into generic currency
					// Metadata/Items/Currency/CurrencyStrongboxQualityInfused
				}
			}
			else if (dir == "Delve") {
				const auto filename = walker.filename();
				if (utility::starts_with(filename, "DelveSocketableCurrency"))
					continue; // ignore old resonators

				if (utility::starts_with(filename, "DelveStackableSocketableCurrency")) {
					std::optional<resonator> reso = parse_resonator(path, item_entry);
					if (!reso)
						continue;

					resonators.push_back(std::move(*reso));
				}
				else {
					throw_path_error(dir);
				}
			}
			else if (dir == "DivinationCards") {
				std::optional<currency_item> curr_item = parse_currency_item(path, item_entry);
				if (!curr_item)
					continue;

				divination_cards.push_back(std::move(*curr_item));
			}
			else if (dir == "Flasks") {
				std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
				if (!elem_item)
					continue;

				const auto& item_class = item_entry.at("item_class").get_ref<const string_t&>();
				if (item_class == "LifeFlask")
					flasks.life_flasks.push_back(std::move(*elem_item));
				else if (item_class == "ManaFlask")
					flasks.mana_flasks.push_back(std::move(*elem_item));
				else if (item_class == "HybridFlask")
					flasks.life_flasks.push_back(std::move(*elem_item));
				else if (item_class == "UtilityFlask")
					flasks.utility_flasks.push_back(std::move(*elem_item));
				else if (item_class == "UtilityFlaskCritical")
					flasks.critical_utility_flasks.push_back(std::move(*elem_item));
				else
					throw_path_error(dir);
			}
			else if (dir == "Gems") {
				std::optional<gem> gm = parse_gem(path, item_entry);
				if (!gm)
					continue;

				const auto filename = walker.filename();
				if (utility::starts_with(filename, "SkillGem")) {
					if (utility::contains((*gm).name, "Vaal"))
						gems.vaal_active_gems.push_back(std::move(*gm));
					else
						gems.active_gems.push_back(std::move(*gm));
				}
				else if (utility::starts_with(filename, "SupportGem")) {
					if (utility::starts_with((*gm).name, "Awakened"))
						gems.awakened_support_gems.push_back(std::move(*gm));
					else
						gems.support_gems.push_back(std::move(*gm));
				}
				else {
					throw_path_error(dir);
				}
			}
			else if (dir == "Harvest") {
				continue; // skip Harvest items for now
			}
			else if (dir == "Incursion") {
				std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
				if (!elem_item)
					continue;

				const auto& elem_item_name = (*elem_item).name;
				if (elem_item_name == "Stone of Passage")
					incursion_key = std::move(*elem_item);
				else if (elem_item_name == "Flashpowder Keg")
					incursion_bomb = std::move(*elem_item);
				else
					throw_path_error(dir);
			}
			else if (dir == "Jewels") {
				// implementation note: Timeless and Prismatic jewels will be skipped here
				// - they are tagged with "unique_only"
				std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
				if (!elem_item)
					continue;

				const auto filename = walker.filename();
				if (utility::starts_with(filename, "JewelAbyss"))
					jewels.abyss_jewels.push_back(std::move(*elem_item));
				else if (utility::starts_with(filename, "JewelPassiveTreeExpansion"))
					jewels.cluster_jewels.push_back(std::move(*elem_item));
				else
					jewels.generic_jewels.push_back(std::move(*elem_item));
			}
			else if (dir == "Labyrinth") {
				std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
				if (!elem_item)
					continue;

				const auto filename = walker.filename();
				if (utility::contains(filename, "Key"))
					labyrinth_keys.push_back(std::move(*elem_item));
				else if (filename == "OfferingToTheGoddess")
					continue; // skip old offering
				else
					throw_path_error(dir);
			}
			else if (dir == "Leaguestones") {
				std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
				if (!elem_item)
					continue;

				leaguestones.push_back(std::move(*elem_item));
			}
			else if (dir == "MapFragments") {
				const auto filename = walker.filename();
				if (
					utility::starts_with(filename, "BreachFragment") /* skip old Breachstones */
					|| utility::ends_with(filename, "Complete") /* skip complete sets */
					|| utility::starts_with(filename, "ProphecyFragment") /* skip old prophecy keys */
					|| utility::starts_with(filename, "CurrencyProphecyFragment") /* skip - prophecy content was removed in 3.17 */
					|| utility::starts_with(filename, "ShaperFragment") /* skip old Shaper fragments */
					|| utility::starts_with(filename, "VaalFragment") /* skip old Atziri and Ubr Atziri fragments */
					|| filename == "FragmentPantheonFlask" /* skip old Divine Vessel */)
				{
					continue;
				}

				std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
				if (!elem_item)
					continue;

				if (filename == "CurrencyFragmentPantheonFlask") {
					map_fragments.divine_vessel = std::move(*elem_item);
				}
				else if (utility::contains(filename, "VaultKey")) {
					reliquary_keys.push_back(std::move(*elem_item));
				}
				else if (filename == "CurrencyAfflictionFragment") {
					map_fragments.simulacrum = std::move(*elem_item);
				}
				else if (utility::contains(filename, "CurrencyBreachFragment")) {
					// implementation note - upgraded Breachstones also go here
					map_fragments.breachstones.push_back(std::move(*elem_item));
				}
				else if (utility::starts_with(filename, "CurrencyShaperFragment")) {
					map_fragments.shaper_fragments.push_back(std::move(*elem_item));
				}
				else if (utility::starts_with(filename, "CurrencyElderFragment")) {
					map_fragments.elder_fragments.push_back(std::move(*elem_item));
				}
				else if (utility::starts_with(filename, "CurrencyUberElderFragment")) {
					map_fragments.uber_elder_fragments.push_back(std::move(*elem_item));
				}
				else if (utility::starts_with(filename, "CurrencyLegionFragment")) {
					map_fragments.legion_fragments.push_back(std::move(*elem_item));
				}
				else if (utility::starts_with(filename, "CurrencyVaalFragment1")) {
					map_fragments.atziri_fragments.push_back(std::move(*elem_item));
				}
				else if (utility::starts_with(filename, "CurrencyVaalFragment2")) {
					map_fragments.uber_atziri_fragments.push_back(std::move(*elem_item));
				}
				else if (filename == "CurrencyOfferingToTheGoddess") {
					map_fragments.labyrinth_offering = std::move(*elem_item);
				}
				else if (utility::starts_with(filename, "CurrencyOfferingToTheGoddess")) {
					map_fragments.labyrinth_upgraded_offerings.push_back(std::move(*elem_item));
				}
				else {
					throw_path_error(dir);
				}
			}
			else if (dir == "Maps") {
				const auto filename = walker.filename();
				if (!utility::starts_with(filename, "MapWorlds"))
					continue; // skip all maps that do not exist on current Atlas

				if (utility::starts_with(filename, "MapWorldsHarbinger"))
					continue; // skip Harbinger map bases, they do not drop

				std::optional<map> m = parse_map(path, item_entry);
				if (!m)
					continue;

				maps.push_back(std::move(*m));
			}
			else if (dir == "Metamorphosis") {
				std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
				if (!elem_item)
					continue;

				metamorph_parts.push_back(std::move(*elem_item));
			}
			else if (dir == "QuestItems") {
				dir = walker.next_dir();
				if (dir == "MapUpgrades" || dir == "ShaperMemoryFragments" || dir == "SkillBooks")
					continue; // skip legacy Atlas content and Books (they do not drop)

				if (dir == "Labyrinth") {
					std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
					if (!elem_item)
						continue;

					labyrinth_trinkets.push_back(std::move(*elem_item));
				}

				std::optional<elementary_item> q_item = parse_elementary_item(path, item_entry);
				if (!q_item)
					continue;

				quest_items.push_back(std::move(*q_item));
			}
			else if (dir == "Scarabs") {
				std::optional<elementary_item> elem_item = parse_elementary_item(path, item_entry);
				if (!elem_item)
					continue;

				const auto& elem_item_name = (*elem_item).name;
				if (utility::starts_with(elem_item_name, "Winged"))
					map_fragments.winged_scarabs.push_back(std::move(*elem_item));
				else if (utility::ends_with(elem_item_name, "Lure"))
					map_fragments.lures.push_back(std::move(*elem_item));
				else
					map_fragments.ordinary_scarabs.push_back(std::move(*elem_item));
			}
			else if (dir == "UniqueFragments") {
				std::optional<unique_piece> piece = parse_unique_piece(path, item_entry);
				if (!piece)
					continue;

				unique_pieces.push_back(std::move(*piece));
			}
			else {
				throw_path_error(dir);
			}
		}
		catch (const std::exception& e) {
			logger.error() << e.what() << "\n" << log::severity::info << "skipping this metadata item \""
				<< path << "\": " << utility::dump_json(item_entry) << "\n";
		}
	}

	return true;
}

log::message_stream& operator<<(log::message_stream& stream, const item_database& db)
{
	const auto log_opt = [](const auto& optional) {
		if (optional)
			return "loaded";
		else
			return "not loaded";
	};

	return stream << "item metadata:\n"
		"currency:\n"
		"generic orbs: " << db.currency.generic.size() << "\n"
		"generic orb shards: " << db.currency.generic_shards.size() << "\n"
		"conqueror orbs: " << db.currency.conqueror_orbs.size() << "\n"
		"breach blessings: " << db.currency.breach_blessings.size() << "\n"
		"breach splinters: " << db.currency.breach_splinters.size() << "\n"
		"legion splinters: " << db.currency.legion_splinters.size() << "\n"
		"essences: " << db.currency.essences.size() << "\n"
		"fossils: " << db.currency.fossils.size() << "\n"
		"catalysts: " << db.currency.catalysts.size() << "\n"
		"oils: " << db.currency.oils.size() << "\n"
		"delirium orbs: " << db.currency.delirium_orbs.size() << "\n"
		"harbinger scrolls: " << db.currency.harbinger_scrolls.size() << "\n"
		"incursion vials: " << db.currency.incursion_vials.size() << "\n"
		"bestiary nets: " << db.currency.bestiary_nets.size() << "\n"
		"simulacrum splinter: " << log_opt(db.currency.simulacrum_splinter) << "\n"
		"remnant of corruption: " << log_opt(db.currency.remnant_of_corruption) << "\n"
		"bestiary orb: " << log_opt(db.currency.bestiary_orb) << "\n"
		"albino rhoa feather: " << log_opt(db.currency.albino_rhoa_feather) << "\n"
		"equipment:\n"
		"body armours: " << db.equipment.body_armours.size() << "\n"
		"helmets: " << db.equipment.helmets.size() << "\n"
		"gloves: " << db.equipment.gloves.size() << "\n"
		"boots: " << db.equipment.boots.size() << "\n"
		"axes 1h: " << db.equipment.axes_1h.size() << "\n"
		"maces 1h: " << db.equipment.maces_1h.size() << "\n"
		"swords 1h: " << db.equipment.swords_1h.size() << "\n"
		"thrusting swords: " << db.equipment.thrusting_swords.size() << "\n"
		"claws: " << db.equipment.claws.size() << "\n"
		"daggers: " << db.equipment.daggers.size() << "\n"
		"rune daggers: " << db.equipment.rune_daggers.size() << "\n"
		"wands: " << db.equipment.wands.size() << "\n"
		"axes 2h: " << db.equipment.axes_2h.size() << "\n"
		"maces 2h: " << db.equipment.maces_2h.size() << "\n"
		"swords 2h: " << db.equipment.swords_2h.size() << "\n"
		"staves: " << db.equipment.staves.size() << "\n"
		"warstaves: " << db.equipment.warstaves.size() << "\n"
		"bows: " << db.equipment.bows.size() << "\n"
		"shields: " << db.equipment.shields.size() << "\n"
		"quivers: " << db.equipment.quivers.size() << "\n"
		"amulets: " << db.equipment.amulets.size() << "\n"
		"rings: " << db.equipment.rings.size() << "\n"
		"belts: " << db.equipment.belts.size() << "\n"
		"stygian vise: " << log_opt(db.equipment.stygian_vise) << "\n"
		"talismans: " << db.equipment.talismans.size() << "\n"
		"fishing rods: " << db.equipment.fishing_rods.size() << "\n"
		"flasks:\n"
		"life flasks: " << db.flasks.life_flasks.size() << "\n"
		"mana flasks: " << db.flasks.mana_flasks.size() << "\n"
		"hybrid flasks: " << db.flasks.hybrid_flasks.size() << "\n"
		"utility flasks: " << db.flasks.utility_flasks.size() << "\n"
		"critical utility flasks: " << db.flasks.critical_utility_flasks.size() << "\n"
		"jewels:\n"
		"generic jewels: " << db.jewels.generic_jewels.size() << "\n"
		"abyss jewels: " << db.jewels.abyss_jewels.size() << "\n"
		"cluster jewels: " << db.jewels.cluster_jewels.size() << "\n"
		"gems:\n"
		"active gems: " << db.gems.active_gems.size() << "\n"
		"vaal active gems: " << db.gems.active_gems.size() << "\n"
		"support gems: " << db.gems.support_gems.size() << "\n"
		"awakened support gems: " << db.gems.awakened_support_gems.size() << "\n"
		"fragments:\n"
		"ordinary scarabs: " << db.map_fragments.ordinary_scarabs.size() << "\n"
		"winged scarabs: " << db.map_fragments.winged_scarabs.size() << "\n"
		"lures: " << db.map_fragments.lures.size() << "\n"
		"shaper fragments: " << db.map_fragments.shaper_fragments.size() << "\n"
		"elder fragments: " << db.map_fragments.elder_fragments.size() << "\n"
		"uber elder fragments: " << db.map_fragments.uber_elder_fragments.size() << "\n"
		"atziri fragments: " << db.map_fragments.atziri_fragments.size() << "\n"
		"uber atziri fragments: " << db.map_fragments.uber_atziri_fragments.size() << "\n"
		"legion fragments: " << db.map_fragments.legion_fragments.size() << "\n"
		"breachstones: " << db.map_fragments.breachstones.size() << "\n"
		"simulacrum: " << log_opt(db.map_fragments.simulacrum) << "\n"
		"divine vessel: " << log_opt(db.map_fragments.divine_vessel) << "\n"
		"labyrinth offering: " << log_opt(db.map_fragments.labyrinth_offering) << "\n"
		"labyrinth upgraded offerings: " << db.map_fragments.labyrinth_upgraded_offerings.size() << "\n"
		"other:\n"
		"maps: " << db.maps.size() << "\n"
		"quest items: " << db.quest_items.size() << "\n"
		"incubators: " << db.incubators.size() << "\n"
		"resonators: " << db.resonators.size() << "\n"
		"divination cards: " << db.divination_cards.size() << "\n"
		"metamorph parts: " << db.metamorph_parts.size() << "\n"
		"unique pieces: " << db.unique_pieces.size() << "\n"
		"incursion key: " << log_opt(db.incursion_key) << "\n"
		"incursion bomb: " << log_opt(db.incursion_bomb) << "\n"
		"labyrinth keys: " << db.labyrinth_keys.size() << "\n"
		"labyrinth trinkets: " << db.labyrinth_trinkets.size() << "\n"
		"leaguestones: " << db.leaguestones.size() << "\n"
		"reliquary keys: " << db.reliquary_keys.size() << "\n";
}

}
