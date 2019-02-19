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
