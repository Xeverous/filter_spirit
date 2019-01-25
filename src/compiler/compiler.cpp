#include "compiler/compiler.hpp"
#include "compiler/error.hpp"
#include "parser/ast.hpp"
#include "parser/config.hpp"
#include "parser/parser.hpp"
#include "lang/constants.hpp"
#include "print/compile_error.hpp"
#include "print/structure_printer.hpp"
#include "utility/holds_alternative.hpp"
#include "utility/if_constexpr_workaround.hpp"
#include <cassert>
#include <string_view>
#include <utility>

namespace
{

namespace past = fs::parser::ast;

fs::lang::object parser_literal_to_language_object(const past::value_expression& literal)
{
	return boost::apply_visitor([](const auto& literal) -> fs::lang::object {
		using T = std::decay_t<decltype(literal)>;

		if ECLIPSE_CONSTEXPR (std::is_same_v<T, past::boolean_literal>) {
			return fs::lang::boolean{literal.value};
		}
		else if ECLIPSE_CONSTEXPR (std::is_same_v<T, past::integer_literal>) {
			return fs::lang::number{literal.value};
		}
		else if ECLIPSE_CONSTEXPR (std::is_same_v<T, past::rarity_literal>) {
			return fs::lang::rarity{literal.value};
		}
		else if ECLIPSE_CONSTEXPR (std::is_same_v<T, past::shape_literal>) {
			return fs::lang::shape{literal.value};
		}
		else if ECLIPSE_CONSTEXPR (std::is_same_v<T, past::suit_literal>) {
			return fs::lang::suit{literal.value};
		}
		else if ECLIPSE_CONSTEXPR (std::is_same_v<T, past::color_literal>) {
			return literal.to_lang_color();
		}
		else if ECLIPSE_CONSTEXPR (std::is_same_v<T, past::string_literal>) {
			return fs::lang::string{literal.value};
		}
		else if ECLIPSE_CONSTEXPR (std::is_same_v<T, past::identifier>) {
			assert(false);
			return fs::lang::boolean{false};
		}
		else
		{
			assert(false);
			return fs::lang::boolean{false};
		}
	}, literal);
}

// get type's enum from object
[[nodiscard]]
fs::lang::object_type type_of_object(const fs::lang::object& obj)
{
	if (std::holds_alternative<fs::lang::boolean>(obj))
		return fs::lang::object_type::boolean;
	if (std::holds_alternative<fs::lang::number>(obj))
		return fs::lang::object_type::number;
	if (std::holds_alternative<fs::lang::level>(obj))
		return fs::lang::object_type::level;
	if (std::holds_alternative<fs::lang::sound_id>(obj))
		return fs::lang::object_type::sound_id;
	if (std::holds_alternative<fs::lang::volume>(obj))
		return fs::lang::object_type::volume;
	if (std::holds_alternative<fs::lang::rarity>(obj))
		return fs::lang::object_type::rarity;
	if (std::holds_alternative<fs::lang::shape>(obj))
		return fs::lang::object_type::shape;
	if (std::holds_alternative<fs::lang::suit>(obj))
		return fs::lang::object_type::suit;
	if (std::holds_alternative<fs::lang::color>(obj))
		return fs::lang::object_type::color;
	if (std::holds_alternative<fs::lang::group>(obj))
		return fs::lang::object_type::group;
	if (std::holds_alternative<fs::lang::string>(obj))
		return fs::lang::object_type::string;

	assert(false);
	// make a return to avoid UB
	// debug: fire assertion
	// release: return boolean as it is the most likely to be a noisy bug
	return fs::lang::object_type::boolean;
}

/**
 * @tparam WantedType type that is desired to be added
 * @tparam AllowedTypes types allowed for the value, check in specified order
 *
 * if value contains any of allowed types: add, return no error
 * if value does not contain any of allowed types: return type mismatch error with
 * information about expected type and actual one
 */
// use a struct because functions do not allow partial template specialization
// specialize for 1+ parameter pack and empty parameter pack
template <fs::lang::object_type WantedType, typename... AllowedTypes>
struct add_constant_from_allowed_types;

template <fs::lang::object_type WantedType, typename AllowedTypeFirst, typename... AllowedTypesRest>
struct add_constant_from_allowed_types<WantedType, AllowedTypeFirst, AllowedTypesRest...>
{
	[[nodiscard]]
	fs::compiler::error::error_variant operator()(
		const past::identifier& wanted_name,
		const past::object_type_expression& wanted_type,
		const fs::parser::parsed_object& value,
		const fs::parser::lookup_data& lookup_data,
		fs::parser::constants_map& map)
	{
		if (std::holds_alternative<AllowedTypeFirst>(value.value))
		{
			fs::parser::parsed_object obj{
				value.value,
				lookup_data.position_of(wanted_type),
				value.value_origin,
				lookup_data.position_of(wanted_name)
			};
			map.emplace(wanted_name.value, obj);
			return fs::compiler::error::no_error();
		}
		else
		{
			return add_constant_from_allowed_types<WantedType, AllowedTypesRest...>{}(
				wanted_name, wanted_type, value, lookup_data, map);
		}
	}

};

// default case: no types allowed => return an error
template <fs::lang::object_type WantedType>
struct add_constant_from_allowed_types<WantedType>
{
	[[nodiscard]]
	fs::compiler::error::error_variant operator()(
		const past::identifier& wanted_name,
		const past::object_type_expression& wanted_type,
		const fs::parser::parsed_object& value,
		const fs::parser::lookup_data& lookup_data,
		fs::parser::constants_map& /* map */)
	{
		return fs::compiler::error::type_mismatch{
			WantedType,
			type_of_object(value.value),
			value.value_origin,
			value.type_origin
		};
	}

};

/*
 * add constant to the map from supplied value
 *
 * - check that value is valid for the wanted type
 *   - valid: add wanted type to the map - may require to promote value (eg number to volume)
 *   - invalid: immediately return appropriate error
 *
 * This function is a bit of boilerplate, as most types can ONLY be created from their
 * respective literals. To solve the issue, use the magic of templates to supply
 * a parameter pack of allowed types.
 */
[[nodiscard]]
fs::compiler::error::error_variant add_constant_from_value(
	const past::identifier& wanted_name,
	const past::object_type_expression& wanted_type,
	const fs::parser::parsed_object& value,
	const fs::parser::lookup_data& lookup_data,
	fs::parser::constants_map& map)
{
	switch (wanted_type.value)
	{
		case fs::lang::object_type::boolean:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::boolean,
				fs::lang::boolean
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::number:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::number,
				fs::lang::number
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::level:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::level,
				fs::lang::level,
				fs::lang::number // promotion
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::sound_id:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::sound_id,
				fs::lang::sound_id,
				fs::lang::number // promotion
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::volume:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::volume,
				fs::lang::volume,
				fs::lang::number // promotion
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::rarity:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::rarity,
				fs::lang::rarity
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::shape:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::shape,
				fs::lang::shape
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::suit:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::suit,
				fs::lang::suit
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::color:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::color,
				fs::lang::color
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::group:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::group,
				fs::lang::group
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		case fs::lang::object_type::string:
		{
			return add_constant_from_allowed_types<
				fs::lang::object_type::string,
				fs::lang::string
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}
		default:
		{
			assert(false);
			return fs::compiler::error::internal_error();
		}
	}
}

/*
 * core entry point into adding constants
 *
 * task:
 * Add object only if valid, error with appropriate information otherwise; in any case
 * after either successful addition or any error return immediately.
 *
 * flow:
 * - check that name is not already taken and error if so (0)
 *   (it's impossible to have multiple objects with the same name)
 *   (as of now, filter's language has no scoping/name shadowing)
 * - if right side is an identifier (1)
 *   - check for special identifiers (eg RGBBW) (2)
 *     if so, dispatch to adding value of appropriate type
 *   - check for references to other values (3)
 *     - return error if no such object exists (4)
 *     - proceed otherwise, copying value from referenced object (5)
 * - if right side is a value (parsed a literal) (6)
 *   - convert literal to filter's object and proceed
 */
[[nodiscard]]
fs::compiler::error::error_variant add_constant_from_definition(
	const fs::parser::ast::constant_definition& def,
	const fs::parser::lookup_data& lookup_data,
	fs::parser::constants_map& map)
{
	const past::identifier& wanted_name = def.name;
	const past::object_type_expression& wanted_type = def.object_type;
	const past::value_expression& value_expression = def.value;

	const auto wanted_name_it = map.find(wanted_name.value); // C++17: use if (expr; cond)
	if (wanted_name_it != map.end()) // (0)
	{
		const fs::parser::range_type place_of_duplicated_name = lookup_data.position_of(def.name);
		assert(wanted_name_it->second.name_origin);
		const fs::parser::range_type place_of_original_name = *wanted_name_it->second.name_origin;
		return fs::compiler::error::name_already_exists{place_of_duplicated_name, place_of_original_name};
	}

	if (holds_alternative<past::identifier>(value_expression.get())) // (1)
	{
		const auto& identifier = boost::get<past::identifier>(value_expression.get());

		const std::optional<fs::lang::group> group = fs::compiler::identifier_to_group(identifier.value); // (2)
		if (group)
		{
			const fs::parser::parsed_object value{
				parser_literal_to_language_object(value_expression),
				lookup_data.position_of(value_expression),
				lookup_data.position_of(value_expression),
				std::nullopt
			};
			return add_constant_from_allowed_types<
				fs::lang::object_type::group,
				fs::lang::group
				>{}(wanted_name, wanted_type, value, lookup_data, map);
		}

		// else: not a special identifier, search for referenced value then (3)

		const auto it = map.find(identifier.value); // C++17: use if (expr; cond)
		if (it == map.end())
		{
			const fs::parser::range_type place_of_name = lookup_data.position_of(identifier);
			return fs::compiler::error::no_such_name{place_of_name}; // (4)
		}

		// object acquired from identifier expression: copy it's value and type origin
		// but set name origin where identifier was last encountered
		const fs::parser::parsed_object value{
			it->second.value,
			it->second.type_origin,
			lookup_data.position_of(identifier),
			it->second.name_origin
		};
		return add_constant_from_value(wanted_name, wanted_type, value, lookup_data, map); // (5)
	}

	// (6)
	const fs::parser::parsed_object value{ // FIXME duplicated code, see (2)
		parser_literal_to_language_object(value_expression),
		lookup_data.position_of(value_expression),
		lookup_data.position_of(value_expression),
		std::nullopt
	};
	return add_constant_from_value(
		wanted_name,
		wanted_type,
		value,
		lookup_data,
		map);
}

} // namespace

namespace fs::compiler
{

void print_error(const parser::lookup_data& lookup_data, error::error_variant error, std::ostream& error_stream)
{
	print::compile_error(lookup_data, error, error_stream);
}

[[nodiscard]]
std::optional<parser::constants_map> parse_constants(
	const parser::ast::ast_type& ast,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	parser::constants_map map;

	for (const parser::ast::code_line& line : ast)
	{
		if (line.value)
		{
			const error::error_variant error = add_constant_from_definition(*line.value, lookup_data, map);
			if (!std::holds_alternative<error::no_error>(error))
			{
				print_error(lookup_data, error, error_stream);
				return std::nullopt;
			}
		}
	}

	return map;
}

bool semantic_analysis(const parser::ast::ast_type& ast, const parser::lookup_data& lookup_data, std::ostream& error_stream)
{
	std::optional<parser::constants_map> map = parse_constants(ast, lookup_data, error_stream);

	if (!map)
		return false;

	for (const auto& pair : *map)
	{
		error_stream << pair.first << "\n";
	}

	return true;
}

std::optional<lang::group> identifier_to_group(std::string_view identifier)
{
	lang::group gp;

	for (char c : identifier)
	{
		namespace kw = fs::lang::constants::keywords;

		if (c == kw::r)
			++gp.r;
		else if (c == kw::g)
			++gp.g;
		else if (c == kw::b)
			++gp.b;
		else if (c == kw::w)
			++gp.w;
		else
			return std::nullopt;
	}

	return gp;
}

bool compile(std::string file_content, std::ostream& error_stream)
{
	std::optional<std::pair<parser::ast::ast_type, parser::lookup_data>> parse_result = parser::parse(file_content, error_stream);

	if (!parse_result)
		return false;

	auto& ast = (*parse_result).first;
	auto& lookup_data = (*parse_result).second;

	print::structure_printer()(ast);

	if (!semantic_analysis(ast, lookup_data, error_stream))
		return false;

	return true;
}

}
