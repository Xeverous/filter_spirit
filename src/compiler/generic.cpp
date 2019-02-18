#include "compiler/generic.hpp"
#include "lang/keywords.hpp"
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

std::optional<lang::group> identifier_to_group(std::string_view identifier)
{
	lang::group gp;

	for (char c : identifier)
	{
		namespace kw = lang::keywords;

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

[[nodiscard]]
std::variant<lang::object, error::error_variant> expression_to_object(
	const past::value_expression& value_expression,
	const parser::lookup_data& lookup_data,
	const lang::constants_map& map)
{
	using result_type = std::variant<lang::object, error::error_variant>;

	auto literal_to_object = [&](const past::literal_expression& literal) -> result_type
	{
		lang::single_object obj = literal_to_single_object(literal, lookup_data);
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

	auto function_call_to_object = [&](const past::function_call&) -> result_type
	{
		assert(false);
		return error::internal_error_while_parsing_constant{}; // FIXME implement
	};

	auto array_to_object = [&](const past::array_expression& array_expr) -> result_type
	{
		assert(false);
		return result_type{};
	};

	return value_expression.apply_visitor(x3::make_lambda_visitor<result_type>(
		literal_to_object,
		iden_to_object,
		function_call_to_object,
		array_to_object
	));
}

[[nodiscard]]
std::variant<lang::single_object, error::error_variant> construct_single_object_of_type(
	lang::single_object_type wanted_type,
	lang::single_object object,
	parser::range_type object_value_origin,
	parser::range_type object_type_origin)
{
	lang::single_object_type actual_type = lang::type_of_single_object(object);
	if (actual_type == wanted_type)
		return object;

	// check for promotions
	if (actual_type == lang::single_object_type::number)
	{
		if (wanted_type == lang::single_object_type::level)
			return lang::single_object{lang::level{std::get<lang::number>(object).value, object_value_origin}};

		if (wanted_type == lang::single_object_type::sound_id)
			return lang::single_object{lang::sound_id{std::get<lang::number>(object).value}};

		if (wanted_type == lang::single_object_type::volume)
			return lang::single_object{lang::volume{std::get<lang::number>(object).value}};
	}

	return error::type_mismatch_in_assignment{
		wanted_type,
		actual_type,
		object_value_origin,
		object_type_origin
	};
}

[[nodiscard]]
std::variant<lang::single_object, error::error_variant> construct_single_object_of_type(
	lang::single_object_type wanted_type,
	lang::object object)
{
	using result_type = std::variant<lang::single_object, error::error_variant>;

	return std::visit(utility::visitor{
		[&](lang::single_object&& obj) -> result_type
		{
			return construct_single_object_of_type(wanted_type, std::move(obj), object.value_origin, object.type_origin);
		},
		[&](lang::array_object&&) -> result_type
		{
			return error::type_mismatch_in_assignment{
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
	lang::single_object_type inner_array_type,
	lang::object object)
{
	using result_type = std::variant<lang::array_object, error::error_variant>;

	return std::visit(utility::visitor{
		[&](lang::single_object&& obj) -> result_type
		{
			return error::single_object_to_array_assignment{
				lang::object_type(inner_array_type, true),
				lang::type_of_single_object(obj),
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
	lang::object_type wanted_type,
	lang::object&& object)
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

std::variant<lang::action_set, error::error_variant> construct_action_set(
	const std::vector<parser::ast::action>& actions,
	const lang::constants_map& map,
	const parser::lookup_data& lookup_data)
{
	assert(false);
	return lang::action_set{}; // FIXME implement
}

[[nodiscard]]
std::variant<lang::condition_set, error::error_variant> construct_condition_set(
	const std::vector<parser::ast::condition>& conditions,
	const lang::constants_map& map,
	const parser::lookup_data& lookup_data)
{
	lang::condition_set result_conditions;

	for (const past::condition& expr : conditions)
	{
//		using result_type = std::optional<error::error_variant>;
//
//		const auto error = expr.apply_visitor(x3::make_lambda_visitor<result_type>(
//			[&](const parser::ast::item_level_condition& cond) -> result_type
//			{
//				std::variant<lang::level, error::error_variant> result =
//					/* TODO get_val<level>(map, value) */ error::internal_error_while_parsing_constant{};
//
//				if (std::holds_alternative<error::error_variant>(result))
//					return std::get<error::error_variant>(result);
//
//				auto& level = std::get<lang::level>(result);
//				lang::numeric_range_condition nrc(cond.comparison_type.value, level.value, lookup_data.position_of(cond));
//
//				if (result_conditions.item_level) // FIXME try to add to exising one first
//				{
//					return error::duplicate_condition{
//						(*result_conditions.item_level).origin,
//						lookup_data.position_of(cond)
//					};
//				}
//
//				result_conditions.item_level = nrc;
//				return std::nullopt;
//			},
//			[&](const parser::ast::drop_level_condition& cond) -> result_type
//			{
//				std::variant<lang::level, error::error_variant> result =
//					/* TODO get_val<level>(map, value) */ error::internal_error_while_parsing_constant{};
//
//				if (std::holds_alternative<error::error_variant>(result))
//					return std::get<error::error_variant>(result);
//
//				auto& level = std::get<lang::level>(result);
//				lang::numeric_range_condition nrc(cond.comparison_type.value, level.value, lookup_data.position_of(cond));
//
//				if (result_conditions.drop_level) // FIXME try to add to exising one first
//				{
//					return error::duplicate_condition{
//						(*result_conditions.drop_level).origin,
//						lookup_data.position_of(cond)
//					};
//				}
//
//				result_conditions.drop_level = nrc;
//				return std::nullopt;
//			}
//			));
//
//		if (error)
//			return *error;
	}

	return result_conditions;
}

}
