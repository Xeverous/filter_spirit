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
	 *
	 * std::random_device has been removed due to crashes on MinGW (as of late 2020)
	 */
	return std::seed_seq{static_cast<std::seed_seq::result_type>(std::time(nullptr))};
}

// in some functions .size() == 0 is used instead of .empty()
// because std::initializer_list does not have .empty()
template <typename Container>
auto make_index_dist(const Container& c)
{
	FS_ASSERT_MSG(c.size() > 0u, "to form a distribution container must not be empty");
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
auto select_one_element(const std::initializer_list<T>& elements, RandomNumberGenerator& rng) -> const T*
{
	if (elements.size() == 0u)
		return nullptr;

	auto dist = make_index_dist(elements);
	return elements.begin() + dist(rng);
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

bool roll_one_in_n(int n, std::mt19937& rng)
{
	return roll_in_range({1, n}, rng) == 1;
}

int roll_in_limited_range(range r, range limit, std::mt19937& rng)
{
	return roll_in_range({std::clamp(r.min, limit.min, limit.max), std::clamp(r.max, limit.min, limit.max)}, rng);
}

int roll_stack_size(range r, int max_stack_size, std::mt19937& rng)
{
	return roll_in_limited_range(r, {1, max_stack_size}, rng);
}

int roll_gem_level(range r, int max_gem_level, std::mt19937& rng)
{
	return roll_in_limited_range(r, {1, max_gem_level}, rng);
}

int roll_quality(range r, std::mt19937& rng)
{
	return roll_in_limited_range(r, {0, 20}, rng);
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

	boost::container::small_vector<weight_t, 24> sums;
	std::partial_sum(weights_first, weights_last, std::back_inserter(sums));

	if (sums.empty())
		return 0;

	if (sums.back() == 0)
		return 0;

	std::size_t result = 0;
	const weight_t roll = std::uniform_int_distribution<weight_t>(static_cast<weight_t>(1), sums.back())(rng);
	for (weight_t sub_sum : sums) {
		if (roll <= sub_sum)
			return result;

		++result;
	}

	return result;
}

struct equipment_class_selection
{
	std::reference_wrapper<const std::vector<equippable_item>> bases;
	std::string_view class_name;
	std::initializer_list<const char*> name_suffix_strings;
};

equipment_class_selection roll_equipment_class(const equippable_item_database& db, equippable_item_weights weights, std::mt19937& rng)
{
	const std::initializer_list<equipment_class_selection> list = {
		{ std::ref(db.body_armours), item_class_names::eq_body,   rare_item_names::suffixes_eq_body_armours },
		{ std::ref(db.helmets),      item_class_names::eq_helmet, rare_item_names::suffixes_eq_helmets },
		{ std::ref(db.gloves),       item_class_names::eq_gloves, rare_item_names::suffixes_eq_gloves },
		{ std::ref(db.boots),        item_class_names::eq_boots,  rare_item_names::suffixes_eq_boots },

		{ std::ref(db.axes_1h),          item_class_names::eq_axe_1h,          rare_item_names::suffixes_eq_axes },
		{ std::ref(db.maces_1h),         item_class_names::eq_mace_1h,         rare_item_names::suffixes_eq_maces },
		{ std::ref(db.swords_1h),        item_class_names::eq_sword_1h,        rare_item_names::suffixes_eq_swords },
		{ std::ref(db.thrusting_swords), item_class_names::eq_sword_thrusting, rare_item_names::suffixes_eq_swords },
		{ std::ref(db.claws),            item_class_names::eq_claw,            rare_item_names::suffixes_eq_claws },
		{ std::ref(db.daggers),          item_class_names::eq_dagger,          rare_item_names::suffixes_eq_daggers },
		{ std::ref(db.rune_daggers),     item_class_names::eq_rune_dagger,     rare_item_names::suffixes_eq_daggers },
		{ std::ref(db.wands),            item_class_names::eq_wand,            rare_item_names::suffixes_eq_wands },

		{ std::ref(db.axes_2h),   item_class_names::eq_axe_2h,   rare_item_names::suffixes_eq_axes },
		{ std::ref(db.maces_2h),  item_class_names::eq_mace_2h,  rare_item_names::suffixes_eq_maces },
		{ std::ref(db.swords_2h), item_class_names::eq_sword_2h, rare_item_names::suffixes_eq_swords },
		{ std::ref(db.staves),    item_class_names::eq_staff,    rare_item_names::suffixes_eq_staves },
		{ std::ref(db.warstaves), item_class_names::eq_warstaff, rare_item_names::suffixes_eq_staves },
		{ std::ref(db.bows),      item_class_names::eq_bow,      rare_item_names::suffixes_eq_bows },

		// spirit shields have a separate suffix name pool, code elsewhere needs to check and adjust it
		{ std::ref(db.shields), item_class_names::eq_shield, rare_item_names::suffixes_eq_other_shields },
		{ std::ref(db.quivers), item_class_names::eq_quiver, rare_item_names::suffixes_eq_quivers },

		{ std::ref(db.amulets), item_class_names::eq_amulet, rare_item_names::suffixes_eq_amulets },
		{ std::ref(db.rings),   item_class_names::eq_ring,   rare_item_names::suffixes_eq_rings },
		{ std::ref(db.belts),   item_class_names::eq_belt,   rare_item_names::suffixes_eq_belts },

		{ std::ref(db.fishing_rods), item_class_names::eq_fishing_rod, rare_item_names::suffixes_eq_fishing_rods }
	};

	const std::array<int, 24> weights_array = {
		weights.body_armours, weights.helmets, weights.gloves, weights.boots,
		weights.axes_1h, weights.maces_1h, weights.swords_1h, weights.thrusting_swords, weights.claws, weights.daggers, weights.rune_daggers, weights.wands,
		weights.axes_2h, weights.maces_2h, weights.swords_2h, weights.staves, weights.warstaves, weights.body_armours,
		weights.shields, weights.quivers,
		weights.amulets, weights.rings, weights.belts,
		weights.fishing_rods
	};

	const std::size_t index = roll_index_using_weights(weights_array.begin(), weights_array.end(), rng);
	return list.begin()[index];
}

std::optional<lang::influence_type> roll_influence(influence_weights weights, std::mt19937& rng)
{
	const std::array<int, 7> weights_array = {
		weights.none, weights.shaper, weights.elder,
		weights.crusader, weights.redeemer, weights.hunter, weights.warlord
	};

	const std::size_t index = roll_index_using_weights(weights_array.begin(), weights_array.end(), rng);

	if (index == 0)
		return std::nullopt;
	else
		return static_cast<lang::influence_type>(index - 1);
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

socket_color roll_socket_color(int req_str, int req_dex, int req_int, std::mt19937& rng)
{
	/*
	 * Rolling colors has been studied many times, but no one was very sure on their results.
	 * All we know for sure is that colors are *somewhat* proportional to stat requirements.
	 *
	 * Most popular theory is that each socket's color is rolled independently and each
	 * color has its weight equal to respective stat requirement + a fixed constant value.
	 *
	 * Some people also mentioned that the constant might depend on item's item level, but
	 * there is no significant data to support this theory. If it does, the impact is low.
	 *
	 * Some sources:
	 * https://www.pathofexile.com/forum/view-thread/761831
	 * https://siveran.github.io/calc.html
	 */
	constexpr int c = 14;
	const std::array<int, 3> weights = { req_str + c, req_dex + c, req_int + c };
	const int roll = roll_index_using_weights(weights.begin(), weights.end(), rng);

	if (roll == 0)
		return socket_color::r;
	else if (roll == 1)
		return socket_color::g;
	else
		return socket_color::b;
}

socket_info roll_links_and_colors(int sockets, int req_str, int req_dex, int req_int, std::mt19937& rng)
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
			ls.sockets.push_back(roll_socket_color(req_str, req_dex, req_int, rng));

		result.groups.push_back(ls);
	}

	return result;
}

std::string roll_rare_item_name(
	std::string_view base_type,
	std::initializer_list<const char*> suffix_name_pool,
	std::mt19937& rng)
{
	std::initializer_list<const char*> prefix_name_pool = rare_item_names::prefixes;

	// spirit shields have their specific suffix name pool
	if (utility::ends_with(base_type, "Spirit Shield"))
		suffix_name_pool = rare_item_names::suffixes_eq_spirit_shields;
	// and fishing rods can draw from their own prefix pool
	else if (base_type == "Fishing Rod" && roll_one_in_n(2, rng))
		prefix_name_pool = rare_item_names::prefixes_eq_fishing_rods;

	const auto* const prefix = select_one_element(prefix_name_pool, rng);
	FS_ASSERT(prefix != nullptr);
	const auto* const suffix = select_one_element(suffix_name_pool, rng);
	FS_ASSERT(suffix != nullptr);

	std::string result = *prefix;
	result.append(" ");
	result.append(*suffix);
	return result;
}

std::vector<std::string> roll_explicit_mods_impl(int num_mods)
{
	// not very realistic but correctly generating rare item mods can be a separate project on its own...
	std::vector<std::string> result;
	result.reserve(num_mods);

	for (int i = 0; i < num_mods; ++i)
		result.push_back("explicit item mod #" + std::to_string(i + 1));

	return result;
}

std::vector<std::string> roll_explicit_mods_magic(std::mt19937& rng)
{
	// magic items have 50/50 chance for 1 or 2 mods
	if (roll_one_in_n(2, rng))
		return roll_explicit_mods_impl(1);
	else
		return roll_explicit_mods_impl(2);
}

std::vector<std::string> roll_explicit_mods_rare_6(std::mt19937& rng)
{
	// well known data that chances for 4/5/6 mods are: 8/12, 3/12, and 1/12
	const std::array<int, 3> weights = { 8, 3, 1 };
	const std::size_t roll = roll_index_using_weights(weights.begin(), weights.end(), rng);
	return roll_explicit_mods_impl(roll + 4);
}

// the input quantity to this function should already be a result of multiplication
// of quantity bonuses from all sources (player, map, monster rarity, etc)
int roll_number_of_items(double quantity, std::mt19937& rng)
{
	/*
	 * Higher rarity monsters drop more items with higher chance of high rarity. There
	 * is no official data how it actually works. We only know monster bonuses and that
	 * by default normal monsters have 16% chance to drop anything. We can not just
	 * multiply this chance by quantity bonus because there is no good interpretation
	 * if result is higher than 100%. Instead, for each 100% of the bonus we roll
	 * this chance again and for the last remaining X% we roll the chance multiplied by
	 * this X%. This approximates well multiple chances while also not producing the same
	 * number of items every time.
	 */
	constexpr auto base_chance = 0.16;

	int result = 0;
	const auto num_full_rolls = static_cast<int>(quantity);

	for (int i = 0; i < num_full_rolls; ++i)
		if (std::bernoulli_distribution(base_chance)(rng))
			++result;

	const auto last_roll_coefficient = quantity - static_cast<double>(num_full_rolls);
	FS_ASSERT(last_roll_coefficient >= 0.0);
	FS_ASSERT(last_roll_coefficient <  1.0);
	if (std::bernoulli_distribution(base_chance * last_roll_coefficient)(rng))
		++result;

	return result;
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
	result.rarity_ = rarity_;
	// TODO implement color rolling (based on attribute requirements)
	result.sockets = roll_links_and_colors(roll_sockets_amount(itm.max_sockets, item_level, rng), 0, 0, 0, rng);
	return result;
}

// ---- item modifiers ----

void corrupt_gem(item& itm, std::mt19937& rng)
{
	itm.is_corrupted = true;
	// see generator::generate_gems why it supports only 3 of 4 corruption variants
	const int roll = roll_in_range({0, 2}, rng);

	if (roll == 0) // do nothing
		return;

	if (roll == 1) { // +/- 1 level
		if (roll_one_in_n(2, rng)) {
			++itm.gem_level; // no checking: this corruption is allowed to bypass max_gem_level
		}
		else {
			if (itm.gem_level > 1) // prevent vaaling lvl 1 into lvl 0
				--itm.gem_level;
		}
	}
	else if (roll == 2) { // +/- 1-20 quality, clamp to 0-23
		const int value = roll_in_range({1, 20}, rng);

		if (roll_one_in_n(2, rng))
			itm.quality += value;
		else
			itm.quality -= value;

		itm.quality = std::clamp(itm.quality, 0, 23);
	}
}

void identify_unique_piece(item& itm, const unique_piece& piece)
{
	itm.is_identified = true;
	itm.name = piece.piece_name;
}

void corrupt_unique_piece(item& itm, const unique_piece& piece)
{
	itm.is_corrupted = true;
	identify_unique_piece(itm, piece);
}

void identify_equippable_item(item& itm, std::initializer_list<const char*> suffix_name_pool, std::mt19937& rng)
{
	itm.is_identified = true;
	if (itm.rarity_ == rarity_type::magic) {
		itm.explicit_mods = roll_explicit_mods_magic(rng);
	}
	else if (itm.rarity_ == rarity_type::rare) {
		itm.name = roll_rare_item_name(itm.base_type, suffix_name_pool, rng);
		itm.explicit_mods = roll_explicit_mods_rare_6(rng);
	}
}

void corrupt_equippable_item(
	item& itm, std::initializer_list<const char*> suffix_name_pool, int max_item_sockets, std::mt19937& rng)
{
	// corruption of equipment identifies it
	if (!itm.is_identified)
		identify_equippable_item(itm, suffix_name_pool, rng);

	itm.is_corrupted = true;

	/*
	 * Corruption implementation assumptions:
	 * 25% chance to do nothing (other than adding corrupted property)
	 * 25% chance to add an implicit
	 * 25% chance to turn into 6-mod rare with 1/36 chance to make it 6L
	 * 25% chance to color random non-abyss socket white, with 1/10 chance to repeat
	 */
	const int roll = roll_in_range({0, 3}, rng);
	if (roll == 0) // nothing
		return;

	if (roll == 1) { // adding an implicit
		itm.corrupted_mods = 1;
	}
	else if (roll == 2) { // white sockets
		if (itm.sockets.sockets() == 0)
			return; // do nothing, no sockets to change to white

		boost::container::static_vector<int, 6> socket_indexes;
		for (int i = 0; i < itm.sockets.sockets(); ++i) {
			const socket_color color = itm.sockets[i];
			FS_ASSERT_MSG(color != socket_color::d, "equipment should never have delve sockets");

			if (color != socket_color::a)
				socket_indexes.push_back(i);
		}

		if (socket_indexes.empty())
			return; // no non-abyss sockets to change to white

		// change random socket to white (even if it is white already)
		std::shuffle(socket_indexes.begin(), socket_indexes.end(), rng);
		itm.sockets[socket_indexes.back()] = socket_color::w;
		socket_indexes.pop_back();

		// and continue changing more with 1/10 chance
		while (!socket_indexes.empty() && roll_one_in_n(10, rng)) {
			socket_color& color = itm.sockets[socket_indexes.back()];
			FS_ASSERT(color != socket_color::a);
			FS_ASSERT(color != socket_color::d);
			color = socket_color::w;
			socket_indexes.pop_back();
		}
	}
	else if (roll == 3) {
		// turning into 6-mod rare + rerolling links and colors
		// corruption intentionally ignores stat requirements for colors and item level for sockets
		itm.rarity_ = rarity_type::rare;
		itm.explicit_mods = roll_explicit_mods_impl(6);

		if (max_item_sockets == 6 && roll_one_in_n(36, rng)) { // 6L
			itm.sockets.groups.clear();
			lang::linked_sockets ls;
			for (int i = 0; i < 6; ++i)
				ls.sockets.push_back(roll_socket_color(0, 0, 0, rng));
			itm.sockets.groups.push_back(ls);
		}
		else {
			itm.sockets = roll_links_and_colors(
				roll_sockets_amount(max_item_sockets, lang::limits::max_item_level, rng),
				0, 0, 0, rng);
		}
	}
}

[[nodiscard]] item mirror_item(item itm)
{
	itm.is_mirrored = true;
	return itm;
}

// ---- item generators ----

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
		itm.stack_size = roll_stack_size(p.stack_size, curr_item->max_stack_size, rng);
		receiver.on_item(std::move(itm));
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
	itm.gem_level = roll_gem_level(level, gm->max_level, rng);
	itm.quality = roll_quality(quality, rng);
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
		itm.stack_size = roll_stack_size(p.stack_size, card->max_stack_size, _rng);
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
	fill_with_indexes_of_matching_drop_level_items(area_level, db.resonators, _temp_indexes);

	for (int i = 0; i < count; ++i) {
		const resonator* const reso = select_one_element_by_index(db.resonators, _temp_indexes, _rng);
		if (reso == nullptr)
			return;

		item itm = resonator_to_item(*reso);
		itm.stack_size = roll_stack_size(p.stack_size, reso->max_stack_size, _rng);
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
		const unique_piece* const piece = select_one_element(db.unique_pieces, _rng);
		if (piece == nullptr)
			return;

		item itm = elementary_item_to_item(*piece, item_class_names::piece);
		itm.item_level = item_level;
		itm.rarity_ = rarity_type::unique;
		if (roll_percent(chance_to_corrupt, _rng))
			corrupt_unique_piece(itm, *piece);
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
	generate_currency_items(db.currency.generic, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_generic_currency_shards(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.generic_shards, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_conqueror_orbs(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.conqueror_orbs, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_breach_blessings(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.breach_blessings, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_breach_splinters(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.breach_splinters, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_legion_splinters(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.legion_splinters, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_essences(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.essences, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_fossils(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.fossils, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_catalysts(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.catalysts, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_oils(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.oils, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_delirium_orbs(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.delirium_orbs, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_harbinger_scrolls(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.harbinger_scrolls, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_incursion_vials(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.incursion_vials, _temp_indexes, receiver, p, area_level, _rng);
}

void generator::generate_bestiary_nets(const item_database& db, item_receiver& receiver, plurality p, int area_level)
{
	generate_currency_items(db.currency.bestiary_nets, _temp_indexes, receiver, p, area_level, _rng);
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
					generate_gem(db.gems.vaal_active_gems, _temp_indexes, receiver, level, quality, area_level, percent::never(),  true,  true,  _rng);
				else
					generate_gem(db.gems.active_gems,      _temp_indexes, receiver, level, quality, area_level, percent::always(), false, true,  _rng);
			}
			else { // (3b)
				generate_gem(db.gems.active_gems,   _temp_indexes, receiver, level, quality, area_level, percent::never(),  false, true,  _rng);
			}
		}
		else if (*id == id_gems_vaal_active) {
			generate_gem(db.gems.vaal_active_gems,      _temp_indexes, receiver, level, quality, area_level, chance_to_corrupt, true,  true,  _rng);
		}
		else if (*id == id_gems_support) {
			generate_gem(db.gems.support_gems,          _temp_indexes, receiver, level, quality, area_level, chance_to_corrupt, false, false, _rng);
		}
		else if (*id == id_gems_awakened_support) {
			generate_gem(db.gems.awakened_support_gems, _temp_indexes, receiver, level, quality, area_level, chance_to_corrupt, false, false, _rng);
		}
	}
}

void generator::generate_equippable_item(
	const item_database& db,
	item_receiver& receiver,
	range quality,
	int item_level,
	percent chance_to_identify,
	percent chance_to_corrupt,
	percent chance_to_mirror,
	equippable_item_weights class_weights,
	influence_weights infl_weights,
	rarity_type rarity_)
{
	const equipment_class_selection selection = roll_equipment_class(db.equipment, class_weights, _rng);
	const std::optional<lang::influence_type> influence = roll_influence(infl_weights, _rng);

	fill_with_indexes_of_matching_drop_level_items(item_level, selection.bases.get(), _temp_indexes);
	const equippable_item* const eq_item = select_one_element_by_index(selection.bases.get(), _temp_indexes, _rng);
	if (eq_item == nullptr)
		return;

	item itm = equippable_item_to_item(*eq_item, selection.class_name, item_level, rarity_, _rng);
	itm.quality = roll_quality(quality, _rng);
	if (influence) {
		if (*influence == lang::influence_type::shaper)
			itm.influence.shaper = true;
		else if (*influence == lang::influence_type::elder)
			itm.influence.elder = true;
		else if (*influence == lang::influence_type::crusader)
			itm.influence.crusader = true;
		else if (*influence == lang::influence_type::redeemer)
			itm.influence.redeemer = true;
		else if (*influence == lang::influence_type::hunter)
			itm.influence.hunter = true;
		else if (*influence == lang::influence_type::warlord)
			itm.influence.warlord = true;
	}

	if (roll_percent(chance_to_identify, _rng))
		identify_equippable_item(itm, selection.name_suffix_strings, _rng);

	if (roll_percent(chance_to_corrupt, _rng))
		corrupt_equippable_item(itm, selection.name_suffix_strings, eq_item->max_sockets, _rng);

	if (roll_percent(chance_to_mirror, _rng))
		receiver.on_item(mirror_item(itm));

	receiver.on_item(std::move(itm));
}

void generator::generate_equippable_items_fixed_weights(
	const item_database& db,
	item_receiver& receiver,
	range quality,
	range quantity,
	int area_level,
	percent chance_to_identify,
	percent chance_to_corrupt,
	percent chance_to_mirror,
	equippable_item_weights class_weights,
	influence_weights infl_weights,
	rarity_weights r_weights,
	item_level_weights ilvl_weights)
{
	const int count = roll_in_range(quantity, _rng);
	for (int i = 0; i < count; ++i) {
		const std::array<int, 3> weights_rarity = { r_weights.normal, r_weights.magic, r_weights.rare };
		const auto rarity_ = static_cast<rarity_type>(roll_index_using_weights(weights_rarity.begin(), weights_rarity.end(), _rng));

		const std::array<int, 3> weights_ilvl = { ilvl_weights.base, ilvl_weights.plus_one, ilvl_weights.plus_two };
		const int item_level = roll_index_using_weights(weights_ilvl.begin(), weights_ilvl.end(), _rng) + area_level;

		generate_equippable_item(
			db, receiver, quality, item_level,
			chance_to_identify, chance_to_corrupt, chance_to_mirror,
			class_weights, infl_weights, rarity_);
	}
}

void generator::generate_equippable_items_monster_pack(
	const item_database& db,
	item_receiver& receiver,
	range quality,
	range normal_monsters,
	range magic_monsters,
	range rare_monsters,
	range unique_monsters,
	int area_level,
	double rarity,
	percent chance_to_identify,
	percent chance_to_corrupt,
	percent chance_to_mirror,
	equippable_item_weights class_weights,
	influence_weights infl_weights)
{
	_temp_ilvl_rarity.clear();

	const auto roll_ilvl_rarity = [&](range monsters, double quantity, double rarity, int item_level) {
		const int num_monsters = roll_in_range(monsters, _rng);
		for (int i = 0; i < num_monsters; ++i) {
			const int num_items = roll_number_of_items(quantity, _rng);
			for (int j = 0; j < num_items; j++)
				_temp_ilvl_rarity.emplace_back(item_level, roll_non_unique_rarity(rarity, _rng));
		}
	};

	// data on monster IIQ and IIR bonuses from https://pathofexile.gamepedia.com/Rarity#Monsters
	roll_ilvl_rarity(normal_monsters,  1.0, rarity,        area_level);
	roll_ilvl_rarity(magic_monsters,   7.0, rarity *  3.0, area_level + 1);
	roll_ilvl_rarity(rare_monsters,   15.0, rarity * 11.0, area_level + 2);
	roll_ilvl_rarity(unique_monsters, 29.5, rarity * 11.0, area_level + 2);

	// shuffle rolled ilvl and rarity to avoid generating items
	// in increasing order of these 2 properties
	std::shuffle(_temp_ilvl_rarity.begin(), _temp_ilvl_rarity.end(), _rng);

	for (const auto& [ilvl, rar] : _temp_ilvl_rarity) {
		generate_equippable_item(
			db, receiver, quality, ilvl,
			chance_to_identify, chance_to_corrupt, chance_to_mirror,
			class_weights, infl_weights, rar);
	}
}

}
