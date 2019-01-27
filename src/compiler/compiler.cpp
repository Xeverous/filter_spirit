#include "compiler/compiler.hpp"
#include "compiler/generic.hpp"
#include "compiler/error.hpp"
#include "parser/ast.hpp"
#include "parser/config.hpp"
#include "parser/parser.hpp"
#include "print/compile_error.hpp"
#include "print/structure_printer.hpp"
#include "utility/holds_alternative.hpp"
#include <cassert>
#include <string_view>
#include <utility>

namespace
{

namespace past = fs::parser::ast;

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
	std::optional<fs::compiler::error::error_variant> operator()(
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
			return std::nullopt;
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
	std::optional<fs::compiler::error::error_variant> operator()(
		const past::identifier& /* wanted_name */,
		const past::object_type_expression& /* wanted_type */,
		const fs::parser::parsed_object& value,
		const fs::parser::lookup_data& /* lookup_data */,
		fs::parser::constants_map& /* map */)
	{
		return fs::compiler::error::type_mismatch{
			WantedType,
			fs::compiler::type_of_object(value.value),
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
std::optional<fs::compiler::error::error_variant> add_constant_from_value(
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
			// got this error? then the switch has missing a case for some type
			return fs::compiler::error::internal_error_while_parsing_constant {
				lookup_data.position_of(wanted_name),
				lookup_data.position_of(wanted_type),
				value.type_origin,
				value.value_origin,
				value.name_origin
			};
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
std::optional<fs::compiler::error::error_variant> add_constant_from_definition(
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
				*group,
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
	const fs::parser::parsed_object value{
		fs::compiler::parser_literal_to_language_object(value_expression),
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
			const std::optional<fs::compiler::error::error_variant> error = add_constant_from_definition(*line.value, lookup_data, map);
			if (error)
			{
				print_error(lookup_data, *error, error_stream);
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

bool compile(const std::string& file_content, std::ostream& error_stream)
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
