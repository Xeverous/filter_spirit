#pragma once

#include <fs/lang/primitive_types.hpp>
#include <fs/lang/influence_info.hpp>

#include <boost/container/static_vector.hpp>

#include <algorithm>
#include <optional>
#include <numeric>
#include <vector>
#include <string_view>
#include <string>
#include <stdexcept>
#include <initializer_list>

namespace fs::lang
{

// https://www.poewiki.net/wiki/Item_class
namespace item_class_names {

	// core
	constexpr auto currency_stackable         = "Stackable Currency";
	constexpr auto scrolls_generic            = currency_stackable;
	constexpr auto scrolls_harbinger          = currency_stackable;
	constexpr auto remnant_of_corruption      = currency_stackable;
	constexpr auto essences                   = currency_stackable;
	constexpr auto fossils                    = currency_stackable;
	constexpr auto oils                       = currency_stackable;
	constexpr auto catalysts                  = currency_stackable;
	constexpr auto blessings                  = currency_stackable;
	constexpr auto delirium_orbs              = currency_stackable;
	constexpr auto vials                      = currency_stackable;
	constexpr auto nets                       = currency_stackable;
	constexpr auto stacked_deck               = currency_stackable;
	constexpr auto tattoos                    = currency_stackable;
	constexpr auto resonators                 = "Delve Stackable Socketable Currency";
	constexpr auto divination_card            = "Divination Card";
	constexpr auto incubator                  = "Incubator"; // TODO why not plural?
	constexpr auto gems_active                = "Skill Gems";
	constexpr auto gems_support               = "Support Gems";
	constexpr auto quest_items                = "Quest Items";
	constexpr auto labyrinth_item             = "Labyrinth Item";
	constexpr auto labyrinth_trinket          = "Labyrinth Trinket";
	constexpr auto incursion_item             = "Incursion Item";
	constexpr auto harvest_seed               = "Harvest Seed";
	constexpr auto harvest_seed_enhancer      = "Seed Enhancer";
	constexpr auto leaguestones               = "Leaguestones";
	constexpr auto misc_map_items             = "Misc Map Items";
	constexpr auto map_fragments              = "Map Fragments";
	constexpr auto scarabs                    = map_fragments;
	constexpr auto atlas_region_upgrade_item  = "Atlas Region Upgrade Item";
	constexpr auto watchstones                = atlas_region_upgrade_item;
	constexpr auto heist_trinkets             = "Trinkets";
	constexpr auto heist_target               = "Heist Target";
	constexpr auto heist_contract             = "Contract";
	constexpr auto heist_blueprint            = "Blueprint";
	constexpr auto heist_eq_cloak             = "Heist Cloak";
	constexpr auto heist_eq_brooch            = "Heist Brooch";
	constexpr auto heist_eq_tool              = "Heist Tool";
	constexpr auto heist_eq_gear              = "Heist Gear";
	constexpr auto metamorph_sample           = "Metamorph Sample";
	constexpr auto piece                      = "Piece";
	constexpr auto maps                       = "Maps";
	constexpr auto pantheon_soul              = "Pantheon Soul";
	constexpr auto archnemesis_mod            = "Archnemesis Mod";

	// equipment - main parts
	constexpr auto eq_gloves = "Gloves";
	constexpr auto eq_boots = "Boots";
	constexpr auto eq_helmet = "Helmets";
	constexpr auto eq_body = "Body Armours";

	// equipment - 1h
	constexpr auto eq_claw = "Claws";
	constexpr auto eq_dagger = "Daggers";
	constexpr auto eq_rune_dagger = "Rune Daggers";
	constexpr auto eq_sword_1h = "One Hand Swords";
	constexpr auto eq_sword_thrusting = "Thrusting One Hand Swords";
	constexpr auto eq_axe_1h = "One Hand Axes";
	constexpr auto eq_mace_1h = "One Hand Maces";
	constexpr auto eq_sceptre = "Sceptres";
	constexpr auto eq_wand = "Wands";

	// equipment - 2h
	constexpr auto eq_bow = "Bows";
	constexpr auto eq_staff = "Staves";
	constexpr auto eq_warstaff = "Warstaves";
	constexpr auto eq_sword_2h = "Two Hand Swords";
	constexpr auto eq_axe_2h = "Two Hand Axes";
	constexpr auto eq_mace_2h = "Two Hand Maces";
	constexpr auto eq_fishing_rod = "Fishing Rods";

	// equipment - offhand
	constexpr auto eq_quiver = "Quivers";
	constexpr auto eq_shield = "Shields";

	// equipment - jewellery
	constexpr auto eq_amulet = "Amulets";
	constexpr auto eq_ring = "Rings";
	constexpr auto eq_belt = "Belts";

	// jewels
	constexpr auto jewel_abyss   = "Abyss Jewel";
	constexpr auto jewel_basic   = "Jewel";
	constexpr auto jewel_cluster = jewel_basic;

	// flasks
	constexpr auto flask_life = "Life Flasks";
	constexpr auto flask_mana = "Mana Flasks";
	constexpr auto flask_hybrid = "Hybrid Flasks";
	constexpr auto flask_utility = "Utility Flasks";
	constexpr auto flask_utility_critical = "Critical Utility Flasks";

}

// https://pathofexile.gamepedia.com/Rare_Item_Name_Index
// rare items are named using random prefix + suffix strings
// but wiki has no data on Fishing Rod class
namespace rare_item_names {

	/*
	 * keyword inline is present due to:
	 * - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80351
	 * - Clang's behavior to warn about any unused non-inline constexpr object
	 */

	// all rare items use the same pool of prefixes...
	inline constexpr auto prefixes = {
		"Agony", "Apocalypse", "Armageddon", "Beast", "Behemoth", "Blight", "Blood", "Bramble", "Brimstone",
		"Brood", "Carrion", "Cataclysm", "Chimeric", "Corpse", "Corruption", "Damnation", "Death", "Demon",
		"Dire", "Dragon", "Dread", "Doom", "Dusk", "Eagle", "Empyrean", "Fate", "Foe", "Gale", "Ghoul",
		"Gloom", "Glyph", "Golem", "Grim", "Hate", "Havoc", "Honour", "Horror", "Hypnotic", "Kraken", "Loath",
		"Maelstrom", "Mind", "Miracle", "Morbid", "Oblivion", "Onslaught", "Pain", "Pandemonium", "Phoenix",
		"Plague", "Rage", "Rapture", "Rune", "Skull", "Sol", "Soul", "Sorrow", "Spirit", "Storm", "Tempest",
		"Torment", "Vengeance", "Victory", "Viper", "Vortex", "Woe", "Wrath"
	};

	// ...except fishing rods, which can draw from this pool instead
	// /u/poorFishwife: For dataminers: This is from Words.dat with WordslistKey == 1 || 2.
	// The special words seem to have SpawnWeight_TagKeys set to [80, 0].
	inline constexpr auto prefixes_eq_fishing_rods = {
		"Wet", "Cast", "Line", "Tackle", "Lure", "Bait", "Fish", "Breach", "Angler", "Sinker", "Chum", "Gill", "Jig"
	};

	inline constexpr auto suffixes_eq_body_armours = {
		"Carapace", "Cloak", "Coat", "Curtain", "Guardian", "Hide", "Jack", "Keep", "Mantle", "Pelt",
		"Salvation", "Sanctuary", "Shell", "Shelter", "Shroud", "Skin", "Suit", "Veil", "Ward", "Wrap"
	};

	inline constexpr auto suffixes_eq_helmets = {
		"Brow", "Corona", "Cowl", "Crest", "Crown", "Dome", "Glance", "Guardian", "Halo", "Horn", "Keep",
		"Peak", "Salvation", "Shelter", "Star", "Veil", "Visage", "Visor", "Ward"
	};

	inline constexpr auto suffixes_eq_gloves = {
		"Caress", "Claw", "Clutches", "Fingers", "Fist", "Grasp", "Grip", "Hand", "Hold", "Knuckle", "Mitts",
		"Nails", "Palm", "Paw", "Talons", "Touch", "Vise"
	};

	inline constexpr auto suffixes_eq_boots = {
		"Dash", "Goad", "Hoof", "League", "March", "Pace", "Road", "Slippers", "Sole", "Span", "Spark",
		"Spur", "Stride", "Track", "Trail", "Tread", "Urge"
	};

	inline constexpr auto suffixes_eq_axes = {
		"Bane", "Beak", "Bite", "Butcher", "Edge", "Etcher", "Gnash", "Hunger", "Mangler", "Rend", "Roar",
		"Sever", "Slayer", "Song", "Spawn", "Splitter", "Sunder", "Thirst"
	};

	inline constexpr auto suffixes_eq_maces = {
		"Bane", "Mangler", "Roar", "Batter", "Blast", "Blow", "Blunt", "Brand", "Breaker", "Burst", "Crack",
		"Crusher", "Grinder", "Knell", "Ram", "Ruin", "Shatter", "Smasher", "Star", "Thresher", "Wreck"
	};

	inline constexpr auto suffixes_eq_sceptres = {
		"Bane", "Roar", "Blow", "Breaker", "Crack", "Crusher", "Grinder", "Knell", "Ram", "Smasher", "Star",
		"Song", "Call", "Chant", "Cry", "Gnarl", "Spell", "Weaver",
	};

	inline constexpr auto suffixes_eq_staves = {
		"Bane", "Roar", "Song", "Call", "Chant", "Cry", "Gnarl", "Spell", "Weaver", "Beam", "Branch", "Goad",
		"Mast", "Pile", "Pillar", "Pole", "Post", "Spire"
	};

	inline constexpr auto suffixes_eq_swords = {
		"Bane", "Song", "Beak", "Bite", "Edge", "Hunger", "Sever", "Thirst", "Barb", "Fang", "Gutter", "Impaler",
		"Needle", "Razor", "Saw", "Scalpel", "Scratch", "Skewer", "Slicer", "Spike", "Spiker", "Stinger"
	};

	inline constexpr auto suffixes_eq_daggers = {
		"Bane", "Song", "Bite", "Edge", "Hunger", "Sever", "Thirst", "Barb", "Fang", "Gutter", "Impaler",
		"Needle", "Razor", "Scalpel", "Scratch", "Skewer", "Slicer", "Spike", "Stinger", "Etcher"
	};

	inline constexpr auto suffixes_eq_claws = {
		"Bane", "Song", "Bite", "Edge", "Hunger", "Thirst", "Fang", "Gutter", "Impaler", "Needle", "Razor",
		"Scratch", "Skewer", "Slicer", "Spike", "Stinger", "Roar", "Fist", "Talons"
	};

	inline constexpr auto suffixes_eq_bows = {
		"Bane", "Song", "Thirst", "Stinger", "Branch", "Blast", "Arch", "Barrage", "Breeze", "Fletch", "Guide",
		"Horn", "Mark", "Nock", "Rain", "Reach", "Siege", "Strike", "Thunder", "Twine", "Volley", "Wind", "Wing"
	};

	inline constexpr auto suffixes_eq_wands = {
		"Bane", "Song", "Thirst", "Branch", "Bite", "Edge", "Needle", "Scratch", "Barb", "Call", "Chant",
		"Cry", "Gnarl", "Spell", "Weaver", "Goad", "Spire", "Charm"
	};

	inline constexpr auto suffixes_eq_spirit_shields = {
		"Ancient", "Anthem", "Call", "Chant", "Charm", "Emblem", "Guard", "Mark", "Pith", "Sanctuary",
		"Song", "Spell", "Star", "Ward", "Weaver", "Wish"
	};

	inline constexpr auto suffixes_eq_other_shields = {
		"Aegis", "Badge", "Barrier", "Bastion", "Bulwark", "Duty", "Emblem", "Fend", "Guard", "Mark",
		"Refuge", "Rock", "Rook", "Sanctuary", "Span", "Tower", "Watch", "Wing"
	};

	inline constexpr auto suffixes_eq_quivers = {
		"Arrow", "Barb", "Bite", "Bolt", "Brand", "Dart", "Flight", "Hail", "Impaler", "Nails", "Needle",
		"Quill", "Rod", "Shot", "Skewer", "Spear", "Spike", "Spire", "Stinger"
	};

	inline constexpr auto suffixes_eq_amulets = {
		"Beads", "Braid", "Charm", "Choker", "Clasp", "Collar", "Idol", "Gorget", "Heart", "Locket",
		"Medallion", "Noose", "Pendant", "Rosary", "Scarab", "Talisman", "Torc"
	};

	inline constexpr auto suffixes_eq_rings = {
		"Band", "Circle", "Coil", "Eye", "Finger", "Grasp", "Grip", "Gyre", "Hold", "Knot", "Knuckle", "Loop",
		"Nail", "Spiral", "Turn", "Twirl", "Whorl"
	};

	inline constexpr auto suffixes_eq_belts = {
		"Bind", "Bond", "Buckle", "Clasp", "Cord", "Girdle", "Harness", "Lash", "Leash", "Lock", "Locket",
		"Shackle", "Snare", "Strap", "Tether", "Thread", "Trap", "Twine"
	};

	inline constexpr auto suffixes_eq_fishing_rods = {
		// first line same as fishing rod prefixes
		"Wet", "Cast", "Line", "Tackle", "Lure", "Bait", "Fish", "Breach", "Angler", "Sinker", "Chum", "Gill", "Jig",
		"Barb", "Pole", "Snare", "Doom", "Fate", "Gloom", "Havoc", "Miracle", "Onslaught", "Rage", "Rift", "Rune",
		"Sorrow", "Storm", "Tempest", "Torment", "Vengeance", "Victory", "Woe", "Wrath"
	};

	constexpr auto suffixes_maps = {
		"Abode", "Bind", "Chambers", "Coffers", "Core", "Cradle", "Cramp", "Crest", "Depths", "Dregs",
		"Dwelling", "Frontier", "Haven", "Keep", "Oubliette", "Panorama", "Pit", "Point", "Precinct",
		"Quarters", "Reaches", "Refuge", "Refuse", "Remains", "Roost", "Sanctum", "Scum", "Secrets",
		"Sepulcher", "Shadows", "Solitude", "Trap", "Vault", "View", "Vine", "Waste", "Ziggurat", "Zone"
	};
}

template <typename T, std::size_t N>
using svector = boost::container::static_vector<T, N>;

enum class item_validity
{
	valid,

	identified_without_name,
	unidentified_with_name,
	unidentified_with_explicit_mods,
	synthesised_without_implicit,
	normal_rarity_with_name,
	nonnormal_rarity_without_name,
	non_unique_replica,
	resonator_or_abyss_socket_linked,
	more_than_6_sockets,
	invalid_item_level,
	invalid_drop_level,
	invalid_height,
	invalid_width,
	invalid_quality,
	invalid_stack_size,
	invalid_gem_level,
	invalid_map_tier,
	empty_class,
	empty_base_type
};

enum class socket_color { r, g, b, w, a, d }; // a = abyss, d = resonator (ctrl+C reports resonators with D sockets)

// represents a single group of linked sockets
struct linked_sockets
{
	linked_sockets() = default;

	linked_sockets(std::initializer_list<socket_color> colors)
	{
		for (socket_color c : colors)
			sockets.push_back(c);
	}

	bool is_valid() const noexcept
	{
		return verify() == item_validity::valid;
	}

	item_validity verify() const noexcept
	{
		// resonator and abyss sockets can not be linked with others
		const auto it = std::find_if(sockets.begin(), sockets.end(),
			[](socket_color c){ return c == socket_color::a || c == socket_color::d; });

		if (it != sockets.end() && sockets.size() > 1u)
			return item_validity::resonator_or_abyss_socket_linked;

		return item_validity::valid;
	}

	int count_of(socket_color color) const
	{
		return static_cast<int>(std::count(sockets.begin(), sockets.end(), color));
	}

	int count_red()   const { return count_of(socket_color::r); }
	int count_green() const { return count_of(socket_color::g); }
	int count_blue()  const { return count_of(socket_color::b); }
	int count_white() const { return count_of(socket_color::w); }
	// there is no count A and D because these should never be linked together

	svector<socket_color, 6> sockets;
};

// represents item sockets state
struct socket_info
{
	// return largest group of links
	int links() const noexcept
	{
		int result = 0;

		for (const auto& group : groups)
			result = std::max(result, static_cast<int>(group.sockets.size()));

		return result;
	}

	// return total number of sockets
	int sockets() const noexcept
	{
		return std::accumulate(groups.begin(), groups.end(), 0,
			[](int sum, const linked_sockets& s){ return sum + static_cast<int>(s.sockets.size()); });
	}

	int count_of(socket_color c) const noexcept
	{
		return std::accumulate(groups.begin(), groups.end(), 0,
			[c](int sum, const linked_sockets& s){ return sum + s.count_of(c); });
	}

	bool is_valid() const noexcept
	{
		return verify() == item_validity::valid;
	}

	item_validity verify() const noexcept
	{
		// an item can have at most 6 sockets
		if (sockets() > 6)
			return item_validity::more_than_6_sockets;

		for (linked_sockets group : groups)
			if (auto status = group.verify(); status != item_validity::valid)
				return status;

		return item_validity::valid;
	}

	socket_color& operator[](std::size_t n)
	{
		for (linked_sockets& gr : groups) {
			if (n < gr.sockets.size())
				return gr.sockets[n];
			else
				n -= gr.sockets.size();
		}

		throw std::invalid_argument("invalid socket index: " + std::to_string(n));
	}

	socket_color operator[](std::size_t n) const
	{
		for (const linked_sockets& gr : groups) {
			if (n < gr.sockets.size())
				return gr.sockets[n];
			else
				n -= gr.sockets.size();
		}

		throw std::invalid_argument("invalid socket index: " + std::to_string(n));
	}

	// any item has at most 6 distinct socket groups
	svector<linked_sockets, 6> groups;
};

[[nodiscard]] std::optional<socket_info> to_socket_info(std::string_view str);

[[nodiscard]] char to_char(socket_color color);
[[nodiscard]] std::string to_string(socket_info info);

// traverse sockets and call provided functors
// on_socket should accept (socket_color)
// on_link   should accept (bool)
template <typename OnSocket, typename OnLink>
void traverse_sockets(socket_info info, OnSocket on_socket, OnLink on_link)
{
	bool first_time_group = true;

	for (linked_sockets group : info.groups) {
		if (first_time_group)
			first_time_group = false;
		else
			on_link(false);

		bool first_time_color = true;

		for (socket_color color : group.sockets) {
			if (first_time_color)
				first_time_color = false;
			else
				on_link(true);

			on_socket(color);
		}
	}
}

enum class corruption_status_t { normal, corrupted, scourged };

// none can mean non-blighted map but also not a map item
enum class blight_map_status_t { none, blighted, uber_blighted };

enum class exarch_or_eater_implicit_t { none = 0, lesser = 1, greater = 2, grand = 3, exceptional = 4, exquisite = 5, perfect = 6 };

/**
 * @class describes properties of a dropped item
 */
struct item
{
	bool is_valid() const noexcept
	{
		return verify() == item_validity::valid;
	}

	item_validity verify() const noexcept
	{
		if (name.has_value() && !is_identified)
			return item_validity::unidentified_with_name;

		if (!name.has_value() && is_identified)
			return item_validity::identified_without_name;

		if (name.has_value() && rarity_ == rarity_type::normal)
			return item_validity::normal_rarity_with_name;

		if (!name.has_value() && rarity_ != rarity_type::normal)
			return item_validity::nonnormal_rarity_without_name;

		if (height <= 0)
			return item_validity::invalid_height;

		if (width <= 0)
			return item_validity::invalid_width;

		if (class_.empty())
			return item_validity::empty_class;

		if (base_type.empty())
			return item_validity::empty_base_type;

		// We do not test drop_level vs item_level because some item sources
		// can bypass it (eg. +level strongboxes, divination card exchange).

		// We test quality but given previous breakage with influence (assertion that
		// an item can not be shaper and elder at the same time) it is likely that
		// at some point GGG will introduce negative quality. It could be actually
		// useful for uniques such as the Forbidden Taste.
		if (quality < 0)
			return item_validity::invalid_quality;

		if (auto status = sockets.verify(); status != item_validity::valid)
			return status;

		// An item with non-zero visible explicit mods must be identified.
		if (!explicit_mods.empty() && is_identified == false)
			return item_validity::unidentified_with_explicit_mods;

		if (is_synthesised && !has_non_atlas_implicit_mod)
			return item_validity::synthesised_without_implicit;

		if (stack_size <= 0 || stack_size > max_stack_size)
			return item_validity::invalid_stack_size;

		if (gem_level <= 0) // do not test gem_level > max_gem_level because corruption can bypass it
			return item_validity::invalid_gem_level;

		if (map_tier <= 0)
			return item_validity::invalid_map_tier;

		if (rarity_ != rarity_type::unique && is_replica)
			return item_validity::non_unique_replica;

		return item_validity::valid;
	}

	bool is_corrupted() const { return corruption_status != corruption_status_t::normal; }
	bool is_scourged() const { return corruption_status == corruption_status_t::scourged; }

	int linked_sockets() const { return sockets.links(); }

	bool is_blighted_map() const { return blight_map_status != blight_map_status_t::none; }
	bool is_uber_blighted_map() const { return blight_map_status == blight_map_status_t::uber_blighted; }

	bool has_implicit_mod() const
	{
		return has_non_atlas_implicit_mod
			|| exarch_implicit != exarch_or_eater_implicit_t::none
			|| eater_implicit != exarch_or_eater_implicit_t::none;
	}

	int implicit_exarch_value() const { return static_cast<int>(exarch_implicit); }
	int implicit_eater_value() const { return static_cast<int>(eater_implicit); }

	bool is_shaper_item() const { return influence.shaper; }
	bool is_elder_item() const { return influence.elder; }

	bool has_enchantment() const
	{
		return enchantment_labyrinth.has_value()
			|| enchantment_cluster_jewel.has_value()
			|| !enchantments_other.empty();
	}

	/*
	 * mandatory fields
	 * an item with abnormal values in these will be conidered invalid
	 */
	std::string class_;
	std::optional<std::string> name; // only for IDed items with non-normal rarity
	std::string base_type;
	// these 2 should be overwritten whenever an item is created - size 0x0 makes no sense
	int height = 0;
	int width  = 0;

	/*
	 * "seemingly optional" fields
	 * Many item classes do not seem to have any of these properties.
	 *
	 * In the past, it was important to make a distinction between having any value of the
	 * property and not being able to have it. Item filter blocks could reject an item
	 * because it was impossible to compare lack of property to any value of it. Example:
	 * "Sockets < 6" rejected divination cards because a card could not ever have sockets.
	 *
	 * This was all unintentional, see
	 * https://old.reddit.com/r/pathofexile/comments/f2t4tz/inconsistencies_in_new_filter_syntaxes/fjxf1mx
	 * for disscussion about it. Since then, every item has some default sentinel values (usually 0)
	 * from item filter's point of view.
	 *
	 * Sentinel values below are values which filter considers when an item has no such properties.
	 */
	static constexpr auto sentinel_item_level = 0;
	static constexpr auto sentinel_drop_level = 1;
	static constexpr auto sentinel_quality    = 0;
	static constexpr auto sentinel_rarity     = rarity_type::normal;
	static constexpr auto sentinel_stack_size = 1;
	static constexpr auto sentinel_gem_level  = 0;
	static constexpr auto sentinel_map_tier   = 0;
	static constexpr auto sentinel_base_defence_percentile = 100;
	static constexpr auto sentinel_enchantment_passive_num = 0;
	static constexpr auto sentinel_memory_strands = 0;

	// common and fundamental
	int item_level = sentinel_item_level;
	int drop_level = sentinel_drop_level;
	int quality = sentinel_quality;
	int stack_size = sentinel_stack_size;
	int max_stack_size = sentinel_stack_size; // filters do not support this property
	rarity_type rarity_ = sentinel_rarity;
	socket_info sockets = {};
	int memory_strands = sentinel_memory_strands;

	// defence
	int base_armour = 0;
	int base_evasion = 0;
	int base_energy_shield = 0;
	int base_ward = 0;
	int base_defence_percentile = sentinel_base_defence_percentile;

	// mods - implicit
	corruption_status_t corruption_status = corruption_status_t::normal;
	int corrupted_mods = 0;
	bool has_non_atlas_implicit_mod = false;
	exarch_or_eater_implicit_t exarch_implicit = exarch_or_eater_implicit_t::none;
	exarch_or_eater_implicit_t eater_implicit = exarch_or_eater_implicit_t::none;
	// mods - explicit
	std::vector<std::string> explicit_mods = {};
	// mods - other
	std::optional<std::string> archnemesis_mod;
	influence_info influence = {};

	// enchants
	std::optional<std::string> enchantment_labyrinth;
	std::optional<std::string> enchantment_cluster_jewel = {}; // only the effect, e.g. "Damage over Time"
	std::vector<std::string> enchantments_other = {}; // Annointments and Flask Enchants
	int enchantment_passive_num = sentinel_enchantment_passive_num;

	// gem
	int gem_level = sentinel_gem_level;
	int max_gem_level = sentinel_gem_level; // filters do not support this property

	// map
	int map_tier = sentinel_map_tier;
	blight_map_status_t blight_map_status = blight_map_status_t::none;

	// boolean
	bool is_identified = false;
	bool is_mirrored = false;
	bool is_fractured = false;
	bool is_synthesised = false;
	bool is_shaped_map = false;
	bool is_elder_map = false;
	bool is_replica = false;
	bool has_crucible_passive_tree = false;
	bool is_transfigured_gem = false;
	bool zana_memory = false;

	// A logic-irrelevant field. Only for the user.
	std::string description;
};

int snprintf_dropped_item_label(char* buf, std::size_t buf_size, const item& itm);

}
