#include "compiler/generic.hpp"
#include "lang/constants.hpp"
#include "parser/parser.hpp"
#include "parser/ast.hpp"
#include "utility/visitor.hpp"
#include "utility/holds_alternative.hpp"
#include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp>
#include <cassert>

namespace fs::compiler
{

namespace past = parser::ast;
namespace x3 = boost::spirit::x3;

lang::single_object literal_to_single_object(const past::literal_expression& literal)
{
	return literal.apply_visitor(x3::make_lambda_visitor<lang::single_object>(
		[](past::boolean_literal literal) -> lang::single_object {
			return lang::boolean{literal.value};
		},
		[](past::integer_literal literal) -> lang::single_object {
			return lang::number{literal.value};
		},
		[](past::rarity_literal literal) -> lang::single_object {
			return lang::rarity{literal.value};
		},
		[](past::shape_literal literal) -> lang::single_object {
			return lang::shape{literal.value};
		},
		[](past::suit_literal literal) -> lang::single_object {
			return lang::suit{literal.value};
		},
		[](past::color_literal literal) -> lang::single_object {
			return literal.to_lang_color();
		},
		[](past::string_literal literal) -> lang::single_object {
			return lang::string{literal.value};
		}
	));
}

lang::single_object_type type_of_single_object(const lang::single_object& obj)
{
	if (std::holds_alternative<lang::boolean>(obj))
		return lang::single_object_type::boolean;
	if (std::holds_alternative<lang::number>(obj))
		return lang::single_object_type::number;
	if (std::holds_alternative<lang::level>(obj))
		return lang::single_object_type::level;
	if (std::holds_alternative<lang::sound_id>(obj))
		return lang::single_object_type::sound_id;
	if (std::holds_alternative<lang::volume>(obj))
		return lang::single_object_type::volume;
	if (std::holds_alternative<lang::rarity>(obj))
		return lang::single_object_type::rarity;
	if (std::holds_alternative<lang::shape>(obj))
		return lang::single_object_type::shape;
	if (std::holds_alternative<lang::suit>(obj))
		return lang::single_object_type::suit;
	if (std::holds_alternative<lang::color>(obj))
		return lang::single_object_type::color;
	if (std::holds_alternative<lang::group>(obj))
		return lang::single_object_type::group;
	if (std::holds_alternative<lang::string>(obj))
		return lang::single_object_type::string;

	assert(false);
	// make a return to avoid UB
	// debug: fire assertion
	// release: return boolean as it is the most likely to be a noisy bug
	return lang::single_object_type::boolean;
}

lang::object_type type_expression_to_type(const parser::ast::type_expression& type_expr)
{
	return type_expr.apply_visitor(x3::make_lambda_visitor<lang::object_type>(
		[](const parser::ast::object_type_expression& expr) { return lang::object_type{expr.value}; },
		[](const parser::ast::array_type_expression& expr) { return lang::object_type{expr.value, true}; }
	));
}

lang::object_type type_of_object(const lang::object& obj)
{
	return std::visit(utility::visitor{
		[](const lang::single_object& obj) { return lang::object_type(type_of_single_object(obj)); },
		[](const lang::array_object& obj)
		{
			if (obj.empty())
				return lang::object_type(lang::single_object_type::generic, true);

			return lang::object_type(type_of_single_object(obj.front()), true);
		}
	}, obj.value);
}

std::optional<lang::group> identifier_to_group(std::string_view identifier)
{
	lang::group gp;

	for (char c : identifier)
	{
		namespace kw = lang::constants::keywords;

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

std::optional<error::non_homogeneous_array> verify_homogeneity(
	const lang::array_object& array,
	const std::vector<parser::range_type>& origins)
{
	assert(!array.empty());
	assert(array.size() == origins.size());

	std::size_t first_index = array.front().index();
	for (int i = 1; i < static_cast<int>(array.size()); ++i)
	{
		const auto idx = array[i].index();
		if (idx != first_index) // C++20: [[unlikely]]
		{
			return error::non_homogeneous_array{
				origins.front(),
				origins[i],
				type_of_single_object(array.front()),
				type_of_single_object(array[i])
			};
		}
	}

	return std::nullopt;
}

/*
 * - check for special identifiers (eg RGBBW) (1)
 *   if so, return object of appropriate type
 * - check for references to other values (2)
 *   - return error if no such object exists (3)
 *   - proceed otherwise, copying value from referenced object (4)
 */
[[nodiscard]]
std::variant<lang::object, error::error_variant> identifier_to_object(
	const past::identifier& identifier,
	parser::range_type position_of_identifier,
	const constants_map& map)
{
	const std::optional<lang::group> group = identifier_to_group(identifier.value); // (1)
	if (group)
	{
		return lang::object{
			lang::single_object{*group},
			position_of_identifier,
			position_of_identifier,
			std::nullopt
		};

	}

	// else: not a special identifier, search for referenced value then (2)

	const auto it = map.find(identifier.value); // C++17: use if (expr; cond)
	if (it == map.end())
	{
		return error::no_such_name{position_of_identifier}; // (3)
	}

	// object acquired from identifier expression: copy it's value and type origin
	// but set name origin where identifier was last encountered (4)
	return lang::object{
		it->second.value,
		it->second.type_origin,
		position_of_identifier,
		it->second.name_origin
	};
}

[[nodiscard]]
std::variant<lang::object, error::error_variant> expression_to_object(
	const past::value_expression& value_expression,
	const parser::lookup_data& lookup_data,
	const compiler::constants_map& map)
{
	using result_type = std::variant<lang::object, error::error_variant>;

	auto literal_to_object = [&](const past::literal_expression& literal) -> result_type
	{
		lang::single_object obj = literal_to_single_object(literal);
		return lang::object{
			obj,
			lookup_data.position_of(literal),
			lookup_data.position_of(literal),
			std::nullopt
		};
	};

	auto iden_to_object = [&](const past::identifier& identifier) -> result_type
	{
		return identifier_to_object(identifier, lookup_data.position_of(identifier), map);
	};

	auto array_to_object = [&](const past::array_expression& array_expr) -> result_type
	{
		if (array_expr.values.empty()) // C++20: [[unlikely]]
		{
			return lang::object{
				lang::array_object{},
				lookup_data.position_of(array_expr),
				lookup_data.position_of(array_expr),
				std::nullopt
			};
		}

		// sorry dawg, no recursion this time
		// we don't support nested arrays (at least for now)
		lang::array_object array;
		std::vector<parser::range_type> origins;
		for (const past::value_expression& expr : array_expr.values)
		{
			result_type result = expr.apply_visitor(x3::make_lambda_visitor<result_type>(
				literal_to_object,
				iden_to_object,
				[&lookup_data](const past::array_expression& array_expr) -> result_type
				{
					return error::nested_arrays_not_allowed{lookup_data.position_of(array_expr)};
				}
			));

			if (std::holds_alternative<error::error_variant>(result))
				return std::get<error::error_variant>(result);

			auto& obj = std::get<lang::object>(result);

			if (std::holds_alternative<lang::array_object>(obj.value))
				return error::nested_arrays_not_allowed{lookup_data.position_of(expr)};

			array.push_back(std::get<lang::single_object>(std::move(obj.value)));
			origins.push_back(lookup_data.position_of(expr));
		}

		std::optional<error::non_homogeneous_array> err = verify_homogeneity(array, origins);
		if (err)
			return *err;

		return lang::object{
			std::move(array),
			lookup_data.position_of(array_expr),
			lookup_data.position_of(array_expr),
			std::nullopt
		};
	};

	return value_expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		literal_to_object,
		iden_to_object,
		array_to_object
	));
}

[[nodiscard]]
std::variant<lang::single_object, error::error_variant> construct_single_object_of_type(
	fs::lang::single_object_type wanted_type,
	fs::lang::single_object object,
	parser::range_type object_value_origin,
	parser::range_type object_type_origin)
{
	fs::lang::single_object_type actual_type = type_of_single_object(object);
	if (actual_type == wanted_type)
		return object;

	// check for promotions
	if (actual_type == fs::lang::single_object_type::number)
	{
		if (wanted_type == lang::single_object_type::level)
			return lang::single_object{lang::level{std::get<lang::number>(object).value}};

		if (wanted_type == lang::single_object_type::sound_id)
			return lang::single_object{lang::sound_id{std::get<lang::number>(object).value}};

		if (wanted_type == lang::single_object_type::volume)
			return lang::single_object{lang::volume{std::get<lang::number>(object).value}};
	}

	return error::type_mismatch{
		wanted_type,
		actual_type,
		object_value_origin,
		object_type_origin
	};
}

[[nodiscard]]
std::variant<lang::single_object, error::error_variant> construct_single_object_of_type(
	fs::lang::single_object_type wanted_type,
	fs::lang::object object)
{
	using result_type = std::variant<lang::single_object, error::error_variant>;

	return std::visit(utility::visitor{
		[&](lang::single_object&& obj) -> result_type
		{
			return construct_single_object_of_type(wanted_type, std::move(obj), object.value_origin, object.type_origin);
		},
		[&](lang::array_object&&) -> result_type
		{
			return error::type_mismatch{
				lang::object_type(wanted_type),
				lang::object_type(lang::single_object_type::generic, true),
				object.value_origin,
				object.type_origin
			};
		}
	}, std::move(object.value));
}

[[nodiscard]]
std::variant<lang::array_object, error::error_variant> construct_array_object_of_type(
	fs::lang::single_object_type inner_array_type,
	fs::lang::object object)
{
	using result_type = std::variant<lang::array_object, error::error_variant>;

	return std::visit(utility::visitor{
		[&](lang::single_object&& obj) -> result_type
		{
			return error::single_object_to_array_assignment{
				lang::object_type(inner_array_type, true),
				type_of_single_object(obj),
				object.value_origin
			};
		},
		[&](lang::array_object&& obj) -> result_type
		{
			lang::array_object array;
			for (lang::single_object& single_obj : obj)
			{
				std::variant<lang::single_object, error::error_variant> result =
					construct_single_object_of_type(
						inner_array_type,
						std::move(single_obj),
						object.value_origin,
						object.type_origin);

				if (std::holds_alternative<error::error_variant>(result))
					return std::get<error::error_variant>(result);

				array.push_back(std::get<lang::single_object>(std::move(result)));
			}

			return array;
		}
	}, std::move(object.value));
}

std::variant<lang::object, error::error_variant> construct_object_of_type(
	fs::lang::object_type wanted_type,
	fs::lang::object&& object)
{
	if (wanted_type.is_array)
	{
		std::variant<lang::array_object, error::error_variant> result =
			construct_array_object_of_type(wanted_type.type, std::move(object));

		if (std::holds_alternative<error::error_variant>(result))
			return std::get<error::error_variant>(result);

		return lang::object{
			std::get<lang::array_object>(std::move(result)),
			object.type_origin,
			object.value_origin,
			object.name_origin
		};
	}
	else
	{
		std::variant<lang::single_object, error::error_variant> result =
			construct_single_object_of_type(wanted_type.type, std::move(object));

		if (std::holds_alternative<error::error_variant>(result))
			return std::get<error::error_variant>(result);

		return lang::object{
			std::get<lang::single_object>(std::move(result)),
			object.type_origin,
			object.value_origin,
			object.name_origin
		};
	}
}

}
