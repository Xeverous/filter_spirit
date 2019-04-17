#include "fs/compiler/functions.hpp"
#include "fs/compiler/evaluate_as.hpp"
#include "fs/lang/keywords.hpp"
#include <array>
#include <cassert>
#include <utility>

namespace fs::compiler
{

std::variant<lang::socket_group, error::error_variant> construct_socket_group(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	const int arguments_amount = arguments.size();
	if (arguments_amount != 1)
		return error::invalid_amount_of_arguments{1, 1, arguments_amount, parser::get_position_info(arguments)};

	std::variant<lang::string, error::error_variant> string_or_error = evaluate_as<lang::string>(arguments.front(), map, item_price_data);
	if (std::holds_alternative<error::error_variant>(string_or_error))
		return std::get<error::error_variant>(string_or_error);

	const auto& string = std::get<lang::string>(string_or_error);
	if (string.value.empty())
		return error::empty_socket_group{parser::get_position_info(arguments.front())};

	lang::socket_group sg;
	for (char c : string.value)
	{
		namespace kw = lang::keywords;

		if (c == kw::r)
			++sg.r;
		else if (c == kw::g)
			++sg.g;
		else if (c == kw::b)
			++sg.b;
		else if (c == kw::w)
			++sg.w;
		else
			return error::illegal_characters_in_socket_group{parser::get_position_info(arguments.front())};
	}

	if (!sg.is_valid())
		return error::invalid_socket_group{parser::get_position_info(arguments.front())};

	return sg;
}

std::variant<lang::color, error::error_variant> construct_color(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	const int arguments_amount = arguments.size();
	if (arguments_amount != 3 && arguments_amount != 4)
		return error::invalid_amount_of_arguments{3, 4, arguments_amount, parser::get_position_info(arguments)};

	std::array<int, 3> rgb_values;
	for (std::size_t i = 0; i < rgb_values.size(); ++i)
	{
		std::variant<lang::integer, error::error_variant> int_or_error = evaluate_as<lang::integer>(arguments[i], map, item_price_data);
		if (std::holds_alternative<error::error_variant>(int_or_error))
			return std::get<error::error_variant>(int_or_error);

		const auto& integer = std::get<lang::integer>(int_or_error);
		rgb_values[i] = integer.value;
	}

	if (arguments_amount == 3)
		return lang::color(rgb_values[0], rgb_values[1], rgb_values[2]);

	assert(arguments_amount == 4);
	std::variant<lang::integer, error::error_variant> int_or_error = evaluate_as<lang::integer>(arguments[3], map, item_price_data);
	if (std::holds_alternative<error::error_variant>(int_or_error))
		return std::get<error::error_variant>(int_or_error);

	const auto& integer = std::get<lang::integer>(int_or_error);
	return lang::color(rgb_values[0], rgb_values[1], rgb_values[2], integer.value);
}

std::variant<lang::path, error::error_variant> construct_path(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	const int arguments_amount = arguments.size();
	if (arguments_amount != 1)
		return error::invalid_amount_of_arguments{1, 1, arguments_amount, parser::get_position_info(arguments)};

	std::variant<lang::path, error::error_variant> path_or_error = evaluate_as<lang::path>(arguments.front(), map, item_price_data);
	if (std::holds_alternative<error::error_variant>(path_or_error))
		return std::get<error::error_variant>(path_or_error);

	return std::get<lang::path>(std::move(path_or_error));
}

std::variant<lang::minimap_icon, error::error_variant> construct_minimap_icon(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	const int arguments_amount = arguments.size();
	if (arguments_amount != 3)
		return error::invalid_amount_of_arguments{3, 3, arguments_amount, parser::get_position_info(arguments)};

	std::variant<lang::integer, error::error_variant> integer_or_error = evaluate_as<lang::integer>(arguments[0], map, item_price_data);
	if (std::holds_alternative<error::error_variant>(integer_or_error))
		return std::get<error::error_variant>(integer_or_error);

	const auto& integer = std::get<lang::integer>(integer_or_error);
	if (integer.value != 0 && integer.value != 1 && integer.value != 2)
		return error::invalid_minimap_icon_size{integer.value, parser::get_position_info(arguments[0])};

	std::variant<lang::suit, error::error_variant> suit_or_error = evaluate_as<lang::suit>(arguments[1], map, item_price_data);
	if (std::holds_alternative<error::error_variant>(suit_or_error))
		return std::get<error::error_variant>(suit_or_error);

	std::variant<lang::shape, error::error_variant> shape_or_error = evaluate_as<lang::shape>(arguments[2], map, item_price_data);
	if (std::holds_alternative<error::error_variant>(shape_or_error))
		return std::get<error::error_variant>(shape_or_error);

	return lang::minimap_icon{
		integer.value,
		std::get<lang::suit>(suit_or_error),
		std::get<lang::shape>(shape_or_error)};
}

std::variant<lang::beam_effect, error::error_variant> construct_beam_effect(
	const parser::ast::function_arguments& arguments,
	const lang::constants_map& map,
	const itemdata::item_price_data& item_price_data)
{
	const int arguments_amount = arguments.size();
	if (arguments_amount != 1 && arguments_amount != 2)
		return error::invalid_amount_of_arguments{1, 2, arguments_amount, parser::get_position_info(arguments)};

	std::variant<lang::suit, error::error_variant> suit_or_error = evaluate_as<lang::suit>(arguments[0], map, item_price_data);
	if (std::holds_alternative<error::error_variant>(suit_or_error))
		return std::get<error::error_variant>(suit_or_error);

	const auto& suit = std::get<lang::suit>(suit_or_error);

	if (arguments_amount == 1)
		return lang::beam_effect(suit);
	else
		return error::temporary_beam_effect_not_supported{parser::get_position_info(arguments[1])};
}

}
