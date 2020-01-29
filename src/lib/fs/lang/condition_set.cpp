#include <fs/lang/condition_set.hpp>
#include <fs/lang/keywords.hpp>

#include <cassert>

namespace
{

using namespace fs;
namespace kw = lang::keywords::rf;

std::ostream& operator<<(std::ostream& os, lang::rarity r)
{
	switch (r) {
		case lang::rarity::normal:
			os << kw::normal;
			break;
		case lang::rarity::magic:
			os << kw::magic;
			break;
		case lang::rarity::rare:
			os << kw::rare;
			break;
		case lang::rarity::unique:
			os << kw::unique;
			break;
		default:
			break;
	}

	return os;
}

template <typename T>
void output_range_condition(
	lang::range_condition<T> range,
	const char* name,
	std::ostream& output_stream)
{
	if (!range.has_anything())
		return;

	if (range.is_exact())
	{
		output_stream << '\t' << name << " = " << (*range.lower_bound).value << '\n';
		return;
	}

	if (range.lower_bound.has_value())
	{
		output_stream << '\t' << name << ' ';
		const lang::range_bound<T>& bound = *range.lower_bound;

		if (bound.inclusive)
			output_stream << ">= " << bound.value;
		else
			output_stream << "> " << bound.value;

		output_stream << '\n';
	}

	if (range.upper_bound.has_value())
	{
		output_stream << '\t' << name << ' ';
		const lang::range_bound<T>& bound = *range.upper_bound;

		if (bound.inclusive)
			output_stream << "<= " << bound.value;
		else
			output_stream << "< " << bound.value;

		output_stream << '\n';
	}
}

void output_socket_group_condition(
	std::optional<lang::socket_group_condition> cond,
	std::ostream& output_stream)
{
	if (!cond.has_value())
		return;

	const auto output_letter = [&](char letter, int times)
	{
		for (int i = 0; i < times; ++i)
			output_stream << letter;
	};

	const lang::socket_group& sg = (*cond).group;
	assert(sg.is_valid());

	output_stream << '\t' << kw::socket_group << ' ';
	output_letter(kw::r, sg.r);
	output_letter(kw::g, sg.g);
	output_letter(kw::b, sg.b);
	output_letter(kw::w, sg.w);
	output_stream << '\n';
}

void output_strings_condition(
	const std::optional<lang::strings_condition>& condition,
	const char* name,
	std::ostream& output_stream)
{
	if (!condition.has_value())
		return;

	output_stream << '\t' << name;

	auto& cond = *condition;
	if (cond.exact_match_required)
		output_stream << " ==";

	for (const std::string& str : cond.strings)
		output_stream << " \"" << str << '"';

	output_stream << '\n';
}

void output_influences_condition(
	const std::optional<lang::influences_condition>& condition,
	const char* name,
	std::ostream& output_stream)
{
	if (!condition.has_value())
		return;

	output_stream << '\t' << name;

	auto& cond = *condition;
	if (cond.exact_match_required)
		output_stream << " ==";

	if (cond.shaper)
		output_stream << ' ' << kw::shaper;

	if (cond.elder)
		output_stream << ' ' << kw::elder;

	if (cond.crusader)
		output_stream << ' ' << kw::crusader;

	if (cond.redeemer)
		output_stream << ' ' << kw::redeemer;

	if (cond.hunter)
		output_stream << ' ' << kw::hunter;

	if (cond.warlord)
		output_stream << ' ' << kw::warlord;

	output_stream << '\n';
}

void output_boolean_condition(
	std::optional<lang::boolean_condition> cond,
	const char* name,
	std::ostream& output_stream)
{
	if (!cond.has_value())
		return;

	output_stream << '\t' << name << ' ';

	const lang::boolean_condition& bc = *cond;
	if (bc.value.value)
		output_stream << kw::true_;
	else
		output_stream << kw::false_;

	output_stream << '\n';
}

} // namespace

namespace fs::lang
{

void condition_set::generate(std::ostream& output_stream) const
{
	output_range_condition(item_level, kw::item_level,     output_stream);
	output_range_condition(drop_level, kw::drop_level,     output_stream);
	output_range_condition(quality,    kw::quality,        output_stream);
	output_range_condition(rarity,     kw::rarity,         output_stream);
	output_range_condition(sockets,    kw::sockets,        output_stream);
	output_range_condition(links,      kw::linked_sockets, output_stream);
	output_range_condition(height,     kw::height,         output_stream);
	output_range_condition(width,      kw::width,          output_stream);
	output_range_condition(stack_size, kw::stack_size,     output_stream);
	output_range_condition(gem_level,  kw::gem_level,      output_stream);
	output_range_condition(map_tier,   kw::map_tier,       output_stream);

	output_socket_group_condition(socket_group, output_stream);

	output_boolean_condition(is_identified,       kw::identified,       output_stream);
	output_boolean_condition(is_corrupted,        kw::corrupted,        output_stream);
	output_boolean_condition(is_elder_item,       kw::elder_item,       output_stream);
	output_boolean_condition(is_shaper_item,      kw::shaper_item,      output_stream);
	output_boolean_condition(is_fractured_item,   kw::fractured_item,   output_stream);
	output_boolean_condition(is_synthesised_item, kw::synthesised_item, output_stream);
	output_boolean_condition(is_enchanted,        kw::any_enchantment,  output_stream);
	output_boolean_condition(is_shaped_map,       kw::shaped_map,       output_stream);
	output_boolean_condition(is_elder_map,        kw::elder_map,        output_stream);
	output_boolean_condition(is_blighted_map,     kw::blighted_map,     output_stream);

	output_strings_condition(class_,           kw::class_,           output_stream);
	output_strings_condition(base_type,        kw::base_type,        output_stream);
	output_strings_condition(has_explicit_mod, kw::has_explicit_mod, output_stream);
	output_strings_condition(has_enchantment,  kw::has_enchantment,  output_stream);
	output_strings_condition(prophecy,         kw::prophecy,         output_stream);

	output_influences_condition(has_influence, kw::has_influence, output_stream);
}

bool condition_set::is_valid() const
{
	const auto is_valid_strings_condition = [](const std::optional<strings_condition>& condition) {
		// no condition: ok, we just don't require item to have this property
		if (!condition)
			return true;

		// empty list of allowed values: there are no items that can match this block
		// game client will not accept an empty list so return that the block is invalid
		if ((*condition).strings.empty())
			return false;

		return true;
	};

	const bool influences_valid = [&]() {
		if (!has_influence)
			return true;

		// at least 1 influence must be present
		auto& inf = *has_influence;
		return inf.shaper || inf.elder || inf.crusader || inf.redeemer || inf.hunter || inf.warlord;
	}();

	return influences_valid
		&& is_valid_strings_condition(class_)
		&& is_valid_strings_condition(base_type)
		&& is_valid_strings_condition(has_explicit_mod)
		&& is_valid_strings_condition(has_enchantment)
		&& is_valid_strings_condition(prophecy);
}

}
