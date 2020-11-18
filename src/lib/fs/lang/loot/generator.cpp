#include <fs/lang/loot/generator.hpp>
#include <fs/utility/math.hpp>
#include <fs/utility/assert.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>

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
	FS_ASSERT_MSG(!c.empty(), "to form a distribution container must not be empty");
	return std::uniform_int_distribution<typename Container::size_type>(0, c.size() - 1);
}

template <typename Container, typename RandomNumberGenerator>
auto select_one_element(const Container& elements, RandomNumberGenerator& rng) -> typename Container::const_pointer
{
	if (elements.empty())
		return nullptr;

	auto dist = make_index_dist(elements);
	return &elements[dist(rng)];
}

template <typename T, typename RandomNumberGenerator>
const T* select_one_element_by_index(
	const std::vector<T>& elements,
	const std::vector<std::size_t>& valid_indexes,
	RandomNumberGenerator& rng)
{
	const std::size_t* const index = select_one_element(valid_indexes, rng);
	if (index == nullptr)
		return nullptr;

	FS_ASSERT(*index < elements.size());
	return &elements[*index];
}

template <typename T, typename UnaryPredicate>
void fill_with_indexes_of_matching_items(
	const std::vector<T>& items,
	std::vector<std::size_t>& result,
	UnaryPredicate pred)
{
	result.clear();

	for (std::size_t i = 0; i < items.size(); ++i)
		if (pred(items[i]))
			result.push_back(i);
}

void fill_with_indexes_of_matching_equippable_items(
	int item_level,
	bool allow_atlas_bases,
	const std::vector<equippable_item>& items,
	std::vector<std::size_t>& result)
{
	fill_with_indexes_of_matching_items(items, result, [&](const equippable_item& itm) {
		if (itm.drop_level > item_level)
			return false;

		if (!allow_atlas_bases && itm.is_atlas_base_type)
			return false;

		return true;
	});
}

template <typename T>
void fill_with_indexes_of_matching_drop_level_items(
	int area_level,
	const std::vector<T>& items,
	std::vector<std::size_t>& result)
{
	fill_with_indexes_of_matching_items(items, result, [&](const elementary_item& itm) {
		return itm.drop_level <= area_level;
	});
}

// ---- rarity-related utilities ----

std::pair<const std::vector<equippable_item>&, std::string_view> // set of bases + their class name
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

int roll_in_range(range r, std::mt19937& rng)
{
	if (r.min == r.max)
		return r.min;

	if (r.min > r.max)
		std::swap(r.min, r.max);

	return std::uniform_int_distribution<int>(r.min, r.max)(rng);
}

bool roll_percent(percent chance_to_happen, std::mt19937& rng)
{
	return roll_in_range({percent::min().value + 1, percent::max().value}, rng) <= chance_to_happen.value;
}

rarity_type roll_non_unique_rarity(double rarity, std::mt19937& rng)
{
	if (std::bernoulli_distribution(chance_for_rare(rarity))(rng))
		return rarity_type::rare;

	if (std::bernoulli_distribution(chance_for_magic(rarity))(rng))
		return rarity_type::magic;

	return rarity_type::normal;
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
	FS_ASSERT(max_base_sockets >= 0);
	FS_ASSERT(max_base_sockets <= 6);

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
		const auto idx = roll_index_using_weights(weights.begin(), weights.begin() + sockets, rng);
		const auto linked_together = static_cast<int>(idx) + 1;
		sockets -= linked_together;

		linked_sockets ls;
		for (int i = 0; i < linked_together; ++i)
			ls.sockets.push_back(socket_color::r); // TODO implement color rolling (based on attribute requirements)

		result.groups.push_back(ls);
	}

	return result;
}

int roll_random_quality(std::mt19937& rng)
{
	return std::uniform_int_distribution<int>(0, 20)(rng);
}

// ---- to_item converters ----

item elementary_item_to_item(const currency_item& itm, std::string_view class_) = delete; // prevent overload misuse
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

item currency_item_to_item(const currency_item& itm, std::string_view class_)
{
	item result = elementary_item_to_item(static_cast<const elementary_item&>(itm), class_);
	result.max_stack_size = itm.max_stack_size;
	return result;
}

item divination_card_to_item(const currency_item& card)
{
	return currency_item_to_item(card, item_class_names::divination_card);
}

item resonator_to_item(const resonator& reso)
{
	item result = currency_item_to_item(reso, item_class_names::currency_delve);

	for (int i = 0; i < reso.delve_sockets; ++i)
		result.sockets.groups.push_back(linked_sockets{socket_color::d});

	return result;
}

item gem_to_item(const gem& gm, bool is_active)
{
	item result = elementary_item_to_item(gm, is_active ? item_class_names::gem_active : item_class_names::gem_support);
	result.max_gem_level = gm.max_level;
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
	std::vector<std::size_t>& indexes,
	item_receiver& receiver,
	plurality p,
	int area_level,
	std::mt19937& rng)
{
	const int count = roll_in_range(p.quantity, rng);
	fill_with_indexes_of_matching_drop_level_items(area_level, source, indexes);

	for (int i = 0; i < count; ++i) {
		const currency_item* const curr_item = select_one_element_by_index(source, indexes, rng);
		if (curr_item == nullptr)
			return;

		item itm = currency_item_to_item(*curr_item, item_class_names::currency_stackable);
		itm.stack_size = roll_in_range({p.stack_size.min, std::min(p.stack_size.max, curr_item->max_stack_size)}, rng);
		receiver.on_item(std::move(itm));
	}
}

void corrupt_gem(item& itm, std::mt19937& rng)
{
	itm.is_corrupted = true;
	// see generator::generate_gems why it supports only 3 of 4 corruption variants
	const int roll = roll_in_range({0, 2}, rng);

	// roll == 0 (do nothing)
	if (roll == 1) { // +/- 1 level
		const int add_or_substract = roll_in_range({0, 1}, rng);
		if (add_or_substract == 0) {
			++itm.gem_level; // no checking: this corruption is allowed to bypass max_gem_level
		}
		else {
			if (itm.gem_level > 1) // prevent vaaling lvl 1 into lvl 0
				--itm.gem_level;
		}
	}
	else if (roll == 2) { // +/- 1-20 quality, clamp to 0-23
		const int value = roll_in_range({1, 20}, rng);
		const int sign = roll_in_range({0, 1}, rng);

		if (sign == 0)
			itm.quality += value;
		else
			itm.quality -= value;

		itm.quality = std::clamp(itm.quality, 0, 23);
	}
}

void generate_gem(
	const std::vector<gem>& gems,
	std::vector<std::size_t>& indexes,
	item_receiver& receiver,
	range level,
	range quality,
	int area_level,
	percent chance_to_corrupt,
	bool is_vaal_gem,
	bool is_active,
	std::mt19937& rng)
{
	fill_with_indexes_of_matching_drop_level_items(area_level, gems, indexes);
	const gem* const gm = select_one_element_by_index(gems, indexes, rng);
	if (gm == nullptr)
		return;

	item itm = gem_to_item(*gm, is_active);
	itm.gem_level = roll_in_range({level.min, std::min(level.max, gm->max_level)}, rng);
	itm.quality = roll_in_range(quality, rng);
	if (is_vaal_gem) // vaal gems are always corrupted
		itm.is_corrupted = true;

	if (roll_percent(chance_to_corrupt, rng))
		corrupt_gem(itm, rng);

	receiver.on_item(std::move(itm));
}

} // namespace

namespace fs::lang::loot {

generator::generator()
: _seed_seq(make_seed_seq())
, _rng(_seed_seq)
{
}

void generator::generate_divination_cards(const item_database& db, item_receiver& receiver, plurality p)
{
	const int count = roll_in_range(p.quantity, _rng);
	for (int i = 0; i < count; ++i) {
		const currency_item* card = select_one_element(db.divination_cards, _rng);
		if (card == nullptr)
			return;

		item itm = divination_card_to_item(*card);
		itm.stack_size = roll_in_range({p.stack_size.min, std::min(p.stack_size.max, card->max_stack_size)}, _rng);
		receiver.on_item(std::move(itm));
	}
}

void generator::generate_incubators(const item_database& db, item_receiver& receiver, range quantity, int item_level)
{
	const int count = roll_in_range(quantity, _rng);
	for (int i = 0; i < count; ++i) {
		const elementary_item* const incubator = select_one_element(db.incubators, _rng);
		if (incubator == nullptr)
			return;

		item itm = elementary_item_to_item(*incubator, item_class_names::incubator);
		itm.item_level = item_level;
		receiver.on_item(std::move(itm));
	}
}

void generator::generate_quest_items(const item_database& db, item_receiver& receiver, range quantity)
{
	const int count = roll_in_range(quantity, _rng);
	for (int i = 0; i < count; ++i) {
		const elementary_item* const qi = select_one_element(db.quest_items, _rng);
		if (qi == nullptr)
			return;

		receiver.on_item(elementary_item_to_item(*qi, item_class_names::quest_item));
	}
}

void generator::generate_resonators(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	const int count = roll_in_range(p.quantity, _rng);
	fill_with_indexes_of_matching_drop_level_items(area_level, db.resonators, _sp_indexes);

	for (int i = 0; i < count; ++i) {
		const resonator* const reso = select_one_element_by_index(db.resonators, _sp_indexes, _rng);
		if (reso == nullptr)
			return;

		item itm = resonator_to_item(*reso);
		itm.stack_size = roll_in_range({p.stack_size.min, std::min(p.stack_size.max, reso->max_stack_size)}, _rng);
		receiver.on_item(std::move(itm));
	}
}

void generator::generate_metamorph_parts(const item_database& db, item_receiver& receiver, range quantity, int item_level)
{
	const int count = roll_in_range(quantity, _rng);
	for (int i = 0; i < count; ++i) {
		const elementary_item* const part = select_one_element(db.metamorph_parts, _rng);
		if (part == nullptr)
			return;

		/*
		 * Metamorph parts in game only show 1 line of text but this IS NOT their actual base type name.
		 * The in-game label is eg "Pagan Bishop of Agony's Heart" but base type is eg "Metamorph Heart".
		 * Currently FS does not support such combination (implementation assumes that second line == base type)
		 * so generated metamorph parts will have names equal with the base type.
		 */
		item itm = elementary_item_to_item(*part, item_class_names::metamorph_sample);
		// metamorph parts always drop as identified uniques
		itm.rarity_ = rarity_type::unique;
		itm.is_identified = true;
		itm.item_level = item_level;
		receiver.on_item(std::move(itm));
	}
}

void generator::generate_unique_pieces(const item_database& db, item_receiver& receiver, range quantity, int item_level, percent chance_to_corrupt)
{
	const int count = roll_in_range(quantity, _rng);
	for (int i = 0; i < count; ++i) {
		const elementary_item* const piece = select_one_element(db.unique_pieces, _rng);
		if (piece == nullptr)
			return;

		item itm = elementary_item_to_item(*piece, item_class_names::piece);
		itm.item_level = item_level;
		itm.rarity_ = rarity_type::unique;
		itm.is_corrupted = roll_percent(chance_to_corrupt, _rng);
		receiver.on_item(std::move(itm));
	}
}

void generator::generate_labyrinth_keys(const item_database& db, item_receiver& receiver, range quantity)
{
	const int count = roll_in_range(quantity, _rng);
	for (int i = 0; i < count; ++i) {
		const elementary_item* const key = select_one_element(db.labyrinth_keys, _rng);
		if (key == nullptr)
			return;

		item itm = elementary_item_to_item(*key, item_class_names::labyrinth_item);
		receiver.on_item(std::move(itm));
	}
}

void generator::generate_labyrinth_trinkets(const item_database& db, item_receiver& receiver, range quantity)
{
	const int count = roll_in_range(quantity, _rng);
	for (int i = 0; i < count; ++i) {
		const elementary_item* const trinket = select_one_element(db.labyrinth_trinkets, _rng);
		if (trinket == nullptr)
			return;

		item itm = elementary_item_to_item(*trinket, item_class_names::labyrinth_trinket);
		receiver.on_item(std::move(itm));
	}
}

// currency

void generator::generate_generic_currency(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.generic, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_generic_currency_shards(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.generic_shards, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_conqueror_orbs(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.conqueror_orbs, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_breach_blessings(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.breach_blessings, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_breach_splinters(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.breach_splinters, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_legion_splinters(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.legion_splinters, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_essences(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.essences, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_fossils(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.fossils, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_catalysts(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.catalysts, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_oils(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.oils, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_delirium_orbs(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.delirium_orbs, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_harbinger_scrolls(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.harbinger_scrolls, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_incursion_vials(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.incursion_vials, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_bestiary_nets(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	return generate_currency_items(db.currency.bestiary_nets, _sp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_gems(
	const item_database& db,
	item_receiver& receiver,
	range quantity,
	range level,
	range quality,
	int area_level,
	percent chance_to_corrupt,
	gem_types types)
{
	constexpr int id_gems_active           = 0;
	constexpr int id_gems_vaal_active      = 1;
	constexpr int id_gems_support          = 2;
	constexpr int id_gems_awakened_support = 3;
	boost::container::static_vector<int, 4> gem_type_ids;

	if (types.active)
		gem_type_ids.push_back(id_gems_active);
	if (types.support)
		gem_type_ids.push_back(id_gems_support);
	if (types.vaal_active)
		gem_type_ids.push_back(id_gems_vaal_active);
	if (types.awakened_suport)
		gem_type_ids.push_back(id_gems_awakened_support);

	const int count = roll_in_range(quantity, _rng);
	for (int i = 0; i < count; ++i) {
		const int* const id = select_one_element(gem_type_ids, _rng);
		if (id == nullptr)
			return;

		/*
		 * Gem corruption:
		 * 25% chance to do nothing (only setting corrupted property)
		 * 25% chance for +/- 1 level
		 * 25% chance for +/- 1-20 quality, result clamped to (0-23)%
		 * 25% chance to turn active skill gem into vaal skill gem
		 *
		 * The last outcome is tricky. FS does not have mapping of active-to-vaal so instead:
		 * - generate_gem function only implements 3 of 4 corruption variants.
		 * - (1) If the gem to be generated is an active skill gem:
		 *   - (2) roll corruption chance immediately
		 *     - (3a) If corruption rolled true, roll 25% chance active-to-vaal
		 *       - If roll in step (3a) succeeded, generate vaal   skill gem instead, never corrupt it further
		 *       - If roll in step (3a) failed,    generate active skill gem but always corrupt it
		 *     - (3b) If corruption rolled false,  generate active skill gem but never corrupt it
		 */
		if (*id == id_gems_active) { // (1)
			if (roll_percent(chance_to_corrupt, _rng)) { // (2)
				if (roll_percent({25}, _rng)) // (3a)
					generate_gem(db.gems.vaal_active_gems, _sp_indexes, receiver, level, quality, area_level, percent::never(),  true,  true,  _rng);
				else
					generate_gem(db.gems.active_gems,      _sp_indexes, receiver, level, quality, area_level, percent::always(), false, true,  _rng);
			}
			else { // (3b)
				generate_gem(db.gems.active_gems,   _sp_indexes, receiver, level, quality, area_level, percent::never(),  false, true,  _rng);
			}
		}
		else if (*id == id_gems_vaal_active) {
			generate_gem(db.gems.vaal_active_gems,      _sp_indexes, receiver, level, quality, area_level, chance_to_corrupt, true,  true,  _rng);
		}
		else if (*id == id_gems_support) {
			generate_gem(db.gems.support_gems,          _sp_indexes, receiver, level, quality, area_level, chance_to_corrupt, false, false, _rng);
		}
		else if (*id == id_gems_awakened_support) {
			generate_gem(db.gems.awakened_support_gems, _sp_indexes, receiver, level, quality, area_level, chance_to_corrupt, false, false, _rng);
		}
	}
}

void generator::generate_non_unique_equippable_item(
	const item_database& db,
	item_receiver& receiver,
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
		fill_with_indexes_of_matching_equippable_items(item_level, allow_atlas_bases, bases, _sp_indexes);

		if (_sp_indexes.empty())
			continue;

		const equippable_item* const eq_item = select_one_element_by_index(bases, _sp_indexes, _rng);
		if (eq_item == nullptr)
			continue;

		receiver.on_item(equippable_item_to_item(*eq_item, class_, item_level, rarity_, _rng));
		return;
	}
}

void generator::generate_monster_loot(
	const item_database& db,
	item_receiver& receiver,
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
			generate_non_unique_equippable_item(db, receiver, item_level, rar, allow_atlas_bases);
		}
	}

	const auto last_roll_coefficient = quantity - num_full_rolls;
	if (std::bernoulli_distribution(base_chance * last_roll_coefficient)(_rng)) {
		rarity_type rar = roll_non_unique_rarity(rarity, _rng);
		generate_non_unique_equippable_item(db, receiver, item_level, rar, allow_atlas_bases);
	}
}

void generator::generate_monster_pack_loot(
	const item_database& db,
	item_receiver& receiver,
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
		generate_monster_loot(db, receiver, rarity, quantity, area_level, allow_atlas_bases);

	for (int n = 0; n < num_magic_monsters; ++n)
		generate_monster_loot(db, receiver, rarity * 3.0, quantity * 7.0, area_level + 1, allow_atlas_bases);

	for (int n = 0; n < num_rare_monsters; ++n)
		generate_monster_loot(db, receiver, rarity * 11.0, quantity * 15.0, area_level + 2, allow_atlas_bases);

	for (int n = 0; n < num_unique_monsters; ++n)
		generate_monster_loot(db, receiver, rarity * 11.0, quantity * 29.5, area_level + 2, allow_atlas_bases);
}

}
