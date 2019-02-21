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
	std::optional<lang::socket_group> socket_group,
	std::ostream& output_stream)
{
	if (!socket_group.has_value())
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
		output_stream << " \"" << str << '"';

	output_stream << '\n';
}

void output_boolean_condition(
	std::optional<lang::boolean> boolean,
	const char* name,
	std::ostream& output_stream)
{
	if (!boolean.has_value())
		return;

	output_stream << '\t' << name << ' ';

	const lang::boolean& b = *boolean;
	if (b.value)
		output_stream << lang::generation::true_;
	else
		output_stream << lang::generation::false_;

	output_stream << '\n';
}

}

namespace fs::lang
{

void condition_set::generate(std::ostream& output_stream) const
{
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

	output_boolean_condition(is_identified,  lg::identified,  output_stream);
	output_boolean_condition(is_corrupted,   lg::corrupted,   output_stream);
	output_boolean_condition(is_elder_item,  lg::elder_item,  output_stream);
	output_boolean_condition(is_shaper_item, lg::shaper_item, output_stream);
	output_boolean_condition(is_shaped_map,  lg::shaped_map,  output_stream);

	output_strings_condition(class_,           lg::class_,           output_stream);
	output_strings_condition(base_type,        lg::base_type,        output_stream);
	output_strings_condition(has_explicit_mod, lg::has_explicit_mod, output_stream);
}

bool condition_set::is_valid() const
{
	const auto is_valid_strings_condition = [](const std::shared_ptr<std::vector<std::string>>& sptr)
	{
		// no condition: ok, we just don't require item to have this property
		if (sptr == nullptr)
			return true;

		// empty list of allowed values: there are no items that can match this block
		// game client will not accept an empty list so return that the block is invalid
		if (sptr->empty())
			return false;

		return true;
	};

	return is_valid_strings_condition(class_)
		&& is_valid_strings_condition(base_type)
		&& is_valid_strings_condition(has_explicit_mod);
}

}
