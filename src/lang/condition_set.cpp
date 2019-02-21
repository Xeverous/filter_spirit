#include "lang/condition_set.hpp"
#include "lang/generation.hpp"
#include <cassert>

namespace
{

using namespace fs;

std::ostream& operator<<(std::ostream& os, lang::rarity r)
{
	namespace lg = lang::generation;

	switch (r)
	{
		case lang::rarity::normal:
			os << lg::normal;
			break;
		case lang::rarity::magic:
			os << lg::magic;
			break;
		case lang::rarity::rare:
			os << lg::rare;
			break;
		case lang::rarity::unique:
			os << lg::unique;
			break;
		default:
			break;
	}

	return os;
}

template <typename T>
void output_range_condition(
	std::optional<lang::range_condition<T>> condition,
	const char* name,
	std::ostream& output_stream)
{
	if (!condition.has_value())
		return;

	const lang::range_condition<T>& range = *condition;
	assert(range.lower_bound.has_value() || range.upper_bound.has_value());

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
	std::optional<lang::socket_group> socket_group,
	std::ostream& output_stream)
{
	if (!socket_group)
		return;


	const auto output_letter = [&](char letter, int times)
	{
		for (int i = 0; i < times; ++i)
			output_stream << letter;
	};

	namespace lg = lang::generation;
	const lang::socket_group& sg = *socket_group;
	assert(sg.is_valid());

	output_stream << '\t' << lg::socket_group << ' ';
	output_letter(lg::r, sg.r);
	output_letter(lg::g, sg.g);
	output_letter(lg::b, sg.b);
	output_letter(lg::w, sg.w);
	output_stream << '\n';
}

void output_strings_condition(
	const std::shared_ptr<std::vector<std::string>>& strings_ptr,
	const char* name,
	std::ostream& output_stream)
{
	if (strings_ptr == nullptr)
		return;

	output_stream << '\t' << name;
	for (const std::string& str : *strings_ptr)
		output_stream << ' ' << str;

	output_stream << '\n';
}

}

namespace fs::lang
{

void condition_set::generate(std::ostream& output_stream) const
{
	// order matters for filter performance (first unmatched rule short-circuits entire block)
	// use integer conditions first, string conditions last

	namespace lg = lang::generation;
	output_range_condition(item_level, lg::item_level,     output_stream);
	output_range_condition(drop_level, lg::drop_level,     output_stream);
	output_range_condition(quality,    lg::quality,        output_stream);
	output_range_condition(rarity,     lg::rarity,         output_stream);
	output_range_condition(sockets,    lg::sockets,        output_stream);
	output_range_condition(links,      lg::linked_sockets, output_stream);
	output_range_condition(height,     lg::height,         output_stream);
	output_range_condition(width,      lg::width,          output_stream);
	output_range_condition(stack_size, lg::stack_size,     output_stream);
	output_range_condition(gem_level,  lg::gem_level,      output_stream);
	output_range_condition(map_tier,   lg::map_tier,       output_stream);

	output_socket_group_condition(socket_group, output_stream);

	output_strings_condition(class_,           lg::class_,           output_stream);
	output_strings_condition(base_type,        lg::base_type,        output_stream);
	output_strings_condition(has_explicit_mod, lg::has_explicit_mod, output_stream);
}

}
