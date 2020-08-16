#include <fs/lang/loot/generator.hpp>

#include <ctime>
#include <random>
#include <utility>

namespace {

using namespace fs::lang;
using namespace fs::lang::loot;

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

int roll_stack_size(stack_param stacking, int max_stack_size, std::mt19937& rng)
{
	if (stacking == stack_param::single)
		return 1;
	if (stacking == stack_param::full)
		return max_stack_size;

	std::uniform_int_distribution<int> dist(1, max_stack_size);
	return dist(rng);
}

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

}
