#include <fs/lang/loot/generator.hpp>
#include <fs/utility/math.hpp>
#include <fs/utility/assert.hpp>

#include <boost/container/small_vector.hpp>

#include <ctime>
#include <random>
#include <utility>
#include <initializer_list>
#include <functional>
#include <algorithm>
#include <type_traits>
#include <iterator>

namespace {

using namespace fs;
using namespace fs::lang;
using namespace fs::lang::loot;

// ---- RNG utilities ----

std::seed_seq make_seed_seq()
{
	/*
	 * Some implementations do not implement random_device using
	 * non-deterministic CPU instructions or OS APIs (most notably older
	 * MinGW with GCC < 9.2). Mix it with time-based seed for extra safety.
	 */
	std::random_device rd;
	return std::seed_seq{static_cast<std::random_device::result_type>(std::time(nullptr)), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
}

template <typename Container>
auto make_index_dist(const Container& c)
{
	if (c.empty())
		throw std::invalid_argument("attemped to form a distribution for an empty container");

	return std::uniform_int_distribution<typename Container::size_type>(0, c.size() - 1);
}

template <typename T, typename RandomNumberGenerator>
const T& select_one_element(const std::vector<T>& elements, RandomNumberGenerator& rng)
{
	auto dist = make_index_dist(elements);
	return elements[dist(rng)];
}

template <typename T, typename RandomNumberGenerator>
const T& select_one_element_by_index(
	const std::vector<T>& elements,
	const std::vector<std::size_t>& valid_indexes,
	RandomNumberGenerator& rng)
{
	return elements[select_one_element(valid_indexes, rng)];
}

void indexes_of_matching_equippable_items(
	int item_level,
	bool allow_atlas_bases,
	const std::vector<equippable_item>& items,
	std::vector<std::size_t>& result)
{
	result.clear();

	for (std::size_t i = 0; i < items.size(); ++i) {
		if (items[i].drop_level > item_level)
			continue;

		if (!allow_atlas_bases && items[i].is_atlas_base_type)
			continue;

		result.push_back(i);
	}
}

// ---- rarity-related utilities ----

std::pair<const std::vector<equippable_item>&, std::string_view> // set of bases + theirclass name
select_equipment_kind(
	const equippable_item_database& db,
	std::mt19937& rng)
{
	const std::initializer_list<std::pair<
		std::reference_wrapper<const std::vector<equippable_item>>,
		std::string_view
	>>
	list = {
		{ std::ref(db.body_armours), item_class_names::eq_body },
		{ std::ref(db.helmets),      item_class_names::eq_helmet },
		{ std::ref(db.gloves),       item_class_names::eq_gloves },
		{ std::ref(db.boots),        item_class_names::eq_boots },

		{ std::ref(db.axes_1h),          item_class_names::eq_axe_1h },
		{ std::ref(db.maces_1h),         item_class_names::eq_mace_1h },
		{ std::ref(db.swords_1h),        item_class_names::eq_sword_1h },
		{ std::ref(db.thrusting_swords), item_class_names::eq_sword_thrusting },
		{ std::ref(db.claws),            item_class_names::eq_claw },
		{ std::ref(db.daggers),          item_class_names::eq_dagger },
		{ std::ref(db.rune_daggers),     item_class_names::eq_rune_dagger },
		{ std::ref(db.wands),            item_class_names::eq_wand },

		{ std::ref(db.axes_2h),   item_class_names::eq_axe_2h },
		{ std::ref(db.maces_2h),  item_class_names::eq_mace_2h },
		{ std::ref(db.swords_2h), item_class_names::eq_sword_2h },
		{ std::ref(db.staves),    item_class_names::eq_staff },
		{ std::ref(db.warstaves), item_class_names::eq_warstaff },
		{ std::ref(db.bows),      item_class_names::eq_bow },

		{ std::ref(db.shields), item_class_names::eq_shield },
		{ std::ref(db.quivers), item_class_names::eq_quiver },

		{ std::ref(db.amulets), item_class_names::eq_amulet },
		{ std::ref(db.rings),   item_class_names::eq_ring },
		{ std::ref(db.belts),   item_class_names::eq_belt }
	};

	std::uniform_int_distribution<std::size_t> dist(0, list.size() - 1);
	const auto& selected = list.begin()[dist(rng)];
	return { selected.first.get(), selected.second };
}

/*
 * Rarity and quantity related code based on
 * https://old.reddit.com/r/pathofexile/comments/2xn0xq/analysis_on_the_effectiveness_of_increased_item/
 *
 * Notation: FS code uses normalized values:
 * - default (no IIR/IIQ bonuses) is 1.0
 * - 100% increase means it will be 2.0
 * - 100% decrease means it will be 0.0
 *
 * Used in the implementation:
 * - Rarity bonus seems to linearly scale from +0% to +200%, then changes
 *   to a different linear growth with slower rate between +200% and +400%.
 *   Above +400% IIR, we just continue this rate as there is no better data available.
 * - We add another linear scale in range from -100% to 0% bonus, in order
 *   to drop no rare items when the player has -100% rarity (proven by reddit).
 * - Because /u/tolos was performing the experiment on a boss, the results present
 *   chances for these rarities on a unique monster. To fix this, rarity is divided
 *   first by the unique monster rarity bonus (+1000%).
 */
double chance_for_rare(double rarity)
{
	rarity /= 11.0;

	/*
	 * 0.0 - 1.0 (-100% IIR -    0% IIR) - linear growth from     0% to  6.66%
	 * 1.0 - 3.0 (   0% IIR - +200% IIR) - linear growth from  6.66% to 12.56%
	 * 3.0 - 5.0 (+200% IIR - +400% IIR) - linear growth from 12.56% to 15.57%
	 * 5.0+      (+400% IIR and above) - continue growth in same rate
	 */
	if (rarity <= 1.0)
		return std::max(0.0, rarity * 0.0666);

	if (rarity <= 3.0)
		return utility::lerp(utility::inverse_lerp(rarity, 1.0, 3.0), 0.0666, 0.1256);
	else
		return utility::lerp(utility::inverse_lerp(rarity, 3.0, 5.0), 0.1256, 0.1557);
}

double chance_for_magic(double rarity)
{
	rarity /= 11.0;

	/*
	 * There is a great dispersion in the data of magic items in /u/tolos analysis.
	 * These numbers are rough estimates (based on magic/(magic+normal) ratio) on what
	 * chance an item has to be magic once the rarity roll fails for higher rarities.
	 */
	if (rarity <= 1.0)
		return std::max(0.0, rarity * 0.36);
	if (rarity <= 2.0)
		return utility::lerp(utility::inverse_lerp(rarity, 1.0, 2.0), 0.36, 0.61);
	if (rarity <= 3.0)
		return utility::lerp(utility::inverse_lerp(rarity, 2.0, 3.0), 0.61, 0.78);
	if (rarity <= 4.0)
		return utility::lerp(utility::inverse_lerp(rarity, 3.0, 4.0), 0.78, 0.92);
	if (rarity <= 5.0)
		return utility::lerp(utility::inverse_lerp(rarity, 4.0, 5.0), 0.92, 1.00);
	else
		return 1.0;
}

// ---- rollers ----

rarity_type roll_non_unique_rarity(double rarity, std::mt19937& rng)
{
	if (std::bernoulli_distribution(chance_for_rare(rarity))(rng))
		return rarity_type::rare;

	if (std::bernoulli_distribution(chance_for_magic(rarity))(rng))
		return rarity_type::magic;

	return rarity_type::normal;
}

int roll_stack_size(stack_param stacking, int max_stack_size, std::mt19937& rng)
{
	if (stacking == stack_param::single)
		return 1;
	if (stacking == stack_param::full)
		return max_stack_size;

	std::uniform_int_distribution<int> dist(1, max_stack_size);
	return dist(rng);
}

template <typename Iterator, typename RandomNumberGenerator>
std::size_t roll_index_using_weights(Iterator weights_first, Iterator weights_last, RandomNumberGenerator& rng)
{
	using weight_t = typename std::iterator_traits<Iterator>::value_type;
	static_assert(std::is_integral_v<weight_t>);

	boost::container::small_vector<weight_t, 6> sums;
	std::partial_sum(weights_first, weights_last, std::back_inserter(sums));

	std::size_t result = 0;
	if (sums.empty())
		return result;

	const weight_t roll = std::uniform_int_distribution<weight_t>(static_cast<weight_t>(1), sums.back())(rng);
	for (weight_t sub_sum : sums) {
		if (roll <= sub_sum)
			return result;

		++result;
	}

	return result;
}

int max_sockets_possible(int item_level)
{
	if (item_level < 2)
		return 2;

	if (item_level < 25)
		return 3;

	if (item_level < 35)
		return 4;

	if (item_level < 50)
		return 5;

	return 6;
}

/*
 * First data from reddit about socket chances:
 * https://old.reddit.com/r/pathofexile/comments/269vj5/chances_of_socket_numbers_accidentally_made/chp1zpo/
 * http://www.pathofexile.com/forum/view-thread/20338#p317215
 *
 * number  weight  chance
 * 1         100   32.68%
 * 2          90   29.41%
 * 3          80   26.14%
 * 4          30    9.80%
 * 5           5    1.63%
 * 6           1    0.33%
 *
 * However, this is very old and wiki has a new (probably datamined) data (unfortunately no reference):
 *
 * For slots that can have up to 4 sockets:
 * number  weight  chance
 * 1         100   33.33%
 * 2          90   30.00%
 * 3          80   26.67%
 * 4          30   10.00%
 *
 * For slots that can have up to 3 or 6 sockets:
 * number  weight  chance (3)  chance (6)
 * 1          50     18.52%      16.34%
 * 2         120     44.44%      39.22%
 * 3         100     37.04%      32.68%
 * 4          30       -          9.80%
 * 5           5       -          1.63%
 * 6           1       -          0.33%
 *
 * The code below implements new wiki data.
 */
int roll_sockets_amount(int max_base_sockets, int item_level, std::mt19937& rng)
{
	BOOST_ASSERT(max_base_sockets >= 0);
	BOOST_ASSERT(max_base_sockets <= 6);

	if (max_base_sockets == 0 || max_base_sockets == 1)
		return max_base_sockets;

	// number of sockets is also limited by item level
	const int max_actual = std::min(max_base_sockets, max_sockets_possible(item_level));

	if (max_base_sockets == 4) {
		const std::array<int, 4> weights = { 100, 90, 80, 30 };
		std::size_t idx = roll_index_using_weights(weights.begin(), weights.begin() + max_actual, rng);
		return static_cast<int>(idx) + 1; // add 1 to convert 0-based index into sockets which begin at 1
	}
	else {
		const std::array<int, 6> weights = { 50, 120, 100, 30, 5, 1 };
		std::size_t idx = roll_index_using_weights(weights.begin(), weights.begin() + max_actual, rng);
		return static_cast<int>(idx) + 1; // add 1 to convert 0-based index into sockets which begin at 1
	}
}

socket_info roll_links_and_colors(int sockets, std::mt19937& rng)
{
	socket_info result;
	/*
	 * Mark explains how linking works:
	 * https://www.pathofexile.com/forum/view-thread/73004#p876686
	 *
	 * Probabilities based on empirical data from:
	 * https://www.reddit.com/r/pathofexile/comments/75t1c9/i_used_100000_fusings_for_science_statistics/
	 */
	// multiply probabilities by 1000000000 to create integer weights for the implementation
	const std::array<int, 6> weights = { 178184223, 348177081, 267663189, 196718817, 8256689, 1000000 };

	while (sockets > 0) {
		std::size_t idx = roll_index_using_weights(weights.begin(), weights.begin() + sockets, rng);
		const int linked_together = static_cast<int>(idx) + 1;
		sockets -= linked_together;

		linked_sockets ls;
		for (int i = 0; i < linked_together; ++i)
			ls.sockets.push_back(socket_color::r);

		result.groups.push_back(ls);
	}

	return result;
}

int roll_random_quality(std::mt19937& rng)
{
	return std::uniform_int_distribution<int>(0, 20)(rng);
}

// ---- to_item converters ----

item elementary_item_to_item(const elementary_item& itm, std::string_view class_)
{
	item result;
	result.class_ = class_;
	result.description = "Metadata ID: \"" + itm.metadata_path + "\"";
	// not a mistake: item's **name** in metadata is item's **base_type** for filters
	result.base_type = std::move(itm.name);
	result.drop_level = itm.drop_level;
	result.width = itm.width;
	result.height = itm.height;
	return result;
}

item divination_card_to_item(const currency_item& card)
{
	return elementary_item_to_item(card, item_class_names::divination_card);
}

item resonator_to_item(const resonator& reso)
{
	item result = elementary_item_to_item(reso, item_class_names::currency_delve);

	for (int i = 0; i < reso.delve_sockets; ++i)
		result.sockets.groups.push_back(linked_sockets{socket_color::d});

	return result;
}

item equippable_item_to_item(
	const equippable_item& itm,
	std::string_view class_,
	int item_level,
	rarity_type rarity_,
	std::mt19937& rng)
{
	item result = elementary_item_to_item(itm, class_);
	result.item_level = item_level;
	result.quality = roll_random_quality(rng);
	result.rarity_ = rarity_;
	result.sockets = roll_links_and_colors(roll_sockets_amount(itm.max_sockets, item_level, rng), rng);
	return result;
}

// ---- other ----

void generate_currency_items(
	const std::vector<currency_item>& source,
	item_inserter inserter,
	int count,
	stack_param stacking,
	std::mt19937& rng)
{
	for (int i = 0; i < count; ++i) {
		const currency_item& curr_item = select_one_element(source, rng);
		item itm = elementary_item_to_item(curr_item, item_class_names::currency_stackable);
		itm.stack_size = roll_stack_size(stacking, curr_item.max_stack_size, rng);
		inserter.insert(std::move(itm));
	}
}

}

namespace fs::lang::loot {

generator::generator()
: _seed_seq(make_seed_seq())
, _rng(_seed_seq)
{
}

void generator::generate_cards(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	for (int i = 0; i < count; ++i) {
		const currency_item& card = select_one_element(db.divination_cards, _rng);
		item itm = divination_card_to_item(card);
		itm.stack_size = roll_stack_size(stacking, card.max_stack_size, _rng);
		inserter.insert(std::move(itm));
	}
}

void generator::generate_incubators(const item_database& db, item_inserter inserter, int count, int item_level)
{
	for (int i = 0; i < count; ++i) {
		item itm = elementary_item_to_item(select_one_element(db.incubators, _rng), item_class_names::incubator);
		itm.item_level = item_level;
		inserter.insert(std::move(itm));
	}
}

void generator::generate_quest_items(const item_database& db, item_inserter inserter, int count)
{
	for (int i = 0; i < count; ++i) {
		inserter.insert(elementary_item_to_item(select_one_element(db.quest_items, _rng), item_class_names::quest_item));
	}
}

void generator::generate_resonators(const item_database& db, item_inserter inserter, int count)
{
	for (int i = 0; i < count; ++i) {
		inserter.insert(resonator_to_item(select_one_element(db.resonators, _rng)));
	}
}

void generator::generate_metamorph_parts(const item_database& db, item_inserter inserter, int count)
{
	for (int i = 0; i < count; ++i) {
		item itm = elementary_item_to_item(select_one_element(db.metamorph_parts, _rng), item_class_names::metamorph_sample);
		// metamorph parts always drop as unique
		itm.rarity_ = rarity_type::unique;
		// TODO do meamorph have names?
		// TODO are metamorph IDed?
		inserter.insert(std::move(itm));
	}
}

void generator::generate_unique_pieces(const item_database& db, item_inserter inserter, int count, int item_level)
{
	for (int i = 0; i < count; ++i) {
		item itm = elementary_item_to_item(select_one_element(db.unique_pieces, _rng), item_class_names::piece);
		itm.item_level = item_level;
		itm.rarity_ = rarity_type::unique;
		inserter.insert(std::move(itm));
	}
}

void generator::generate_labyrinth_keys(const item_database& db, item_inserter inserter, int count)
{
	for (int i = 0; i < count; ++i) {
		item itm = elementary_item_to_item(select_one_element(db.labyrinth_keys, _rng), item_class_names::labyrinth_item);
		// TODO do lab keys have ilvl?
		inserter.insert(std::move(itm));
	}
}

void generator::generate_labyrinth_trinkets(const item_database& db, item_inserter inserter, int count)
{
	for (int i = 0; i < count; ++i) {
		item itm = elementary_item_to_item(select_one_element(db.labyrinth_trinkets, _rng), item_class_names::labyrinth_trinket);
		// TODO do lab trinkets have ilvl?
		inserter.insert(std::move(itm));
	}
}

// currency

void generator::generate_generic_currency(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.generic, inserter, count, stacking, _rng);
}

void generator::generate_generic_currency_shards(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	// TODO shards can not be full stack
	return generate_currency_items(db.currency.generic_shards, inserter, count, stacking, _rng);
}

void generator::generate_conqueror_orbs(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.conqueror_orbs, inserter, count, stacking, _rng);
}

void generator::generate_breach_blessings(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.breach_blessings, inserter, count, stacking, _rng);
}

void generator::generate_breach_splinters(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.breach_splinters, inserter, count, stacking, _rng);
}

void generator::generate_legion_splinters(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.legion_splinters, inserter, count, stacking, _rng);
}

void generator::generate_essences(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.essences, inserter, count, stacking, _rng);
}

void generator::generate_fossils(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.fossils, inserter, count, stacking, _rng);
}

void generator::generate_catalysts(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.catalysts, inserter, count, stacking, _rng);
}

void generator::generate_oils(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.oils, inserter, count, stacking, _rng);
}

void generator::generate_delirium_orbs(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.delirium_orbs, inserter, count, stacking, _rng);
}

void generator::generate_harbinger_scrolls(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.harbinger_scrolls, inserter, count, stacking, _rng);
}

void generator::generate_incursion_vials(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.incursion_vials, inserter, count, stacking, _rng);
}

void generator::generate_bestiary_nets(const item_database& db, item_inserter inserter, int count, stack_param stacking)
{
	return generate_currency_items(db.currency.bestiary_nets, inserter, count, stacking, _rng);
}

void generator::generate_non_unique_equippable_item(
	const item_database& db,
	item_inserter inserter,
	int item_level,
	rarity_type rarity_,
	bool allow_atlas_bases)
{
	/*
	 * Kind selection always succeeds but sometimes there might be no valid items
	 * of selected kind for the given item level. Attempt to roll kind again and
	 * give up after a couple of tries. Such implementation is unlikely to match how
	 * the game works but it seems to be a good approximation given no official data.
	 */
	for (int i = 0; i < 10; ++i) {
		const auto [bases, class_] = select_equipment_kind(db.equipment, _rng);
		indexes_of_matching_equippable_items(item_level, allow_atlas_bases, bases, _sp_indexes);

		if (_sp_indexes.empty())
			continue;

		const equippable_item& eq_item = select_one_element_by_index(bases, _sp_indexes, _rng);
		inserter.insert(equippable_item_to_item(eq_item, class_, item_level, rarity_, _rng));
		return;
	}
}

void generator::generate_monster_loot(
	const item_database& db,
	item_inserter inserter,
	double rarity,
	double quantity,
	int item_level,
	bool allow_atlas_bases)
{
	constexpr auto base_chance = 0.16;

	const auto num_full_rolls = static_cast<int>(quantity);
	for (int r = 0; r < num_full_rolls; ++r) {
		if (std::bernoulli_distribution(base_chance)(_rng)) {
			rarity_type rar = roll_non_unique_rarity(rarity, _rng);
			generate_non_unique_equippable_item(db, inserter, item_level, rar, allow_atlas_bases);
		}
	}

	const auto last_roll_coefficient = quantity - num_full_rolls;
	if (std::bernoulli_distribution(base_chance * last_roll_coefficient)(_rng)) {
		rarity_type rar = roll_non_unique_rarity(rarity, _rng);
		generate_non_unique_equippable_item(db, inserter, item_level, rar, allow_atlas_bases);
	}
}

void generator::generate_monster_pack_loot(
	const item_database& db,
	item_inserter inserter,
	double rarity,
	double quantity,
	int area_level,
	bool allow_atlas_bases,
	int num_normal_monsters,
	int num_magic_monsters,
	int num_rare_monsters,
	int num_unique_monsters)
{
	/*
	 * Higher rarity monsters drop more items with higher chance of high rarity. There
	 * is no official data how it actually works. We only know monster bonuses and that
	 * by default normal monsters have 16% chance to drop anything. We can not just
	 * multiply this chance by quantity bonus because there is no good interpretation
	 * if result is higher than 100%. Instead, for each 100% of the bonus we roll
	 * this chance again and for the last remaining X% we roll the chance multiplied by
	 * this X%. This approximates well multiple chances while also not producing the same
	 * number of items every time. Numbers of bonus rarity/quantity per monster from wiki.
	 */
	for (int n = 0; n < num_normal_monsters; ++n)
		generate_monster_loot(db, inserter, rarity, quantity, area_level, allow_atlas_bases);

	for (int n = 0; n < num_magic_monsters; ++n)
		generate_monster_loot(db, inserter, rarity * 3.0, quantity * 7.0, area_level + 1, allow_atlas_bases);

	for (int n = 0; n < num_rare_monsters; ++n)
		generate_monster_loot(db, inserter, rarity * 11.0, quantity * 15.0, area_level + 2, allow_atlas_bases);

	for (int n = 0; n < num_unique_monsters; ++n)
		generate_monster_loot(db, inserter, rarity * 11.0, quantity * 29.5, area_level + 2, allow_atlas_bases);
}

}
