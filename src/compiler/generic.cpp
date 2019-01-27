#include "compiler/generic.hpp"
#include "lang/constants.hpp"
#include "parser/ast.hpp"
#include "utility/visitor.hpp"
#include <cassert>

namespace fs::compiler
{

namespace past = fs::parser::ast;

fs::lang::object parser_literal_to_language_object(const past::value_expression& literal)
{
	return boost::apply_visitor(
		utility::visitor{
			[](past::boolean_literal literal) -> fs::lang::object {
				return fs::lang::boolean{literal.value};
			},
			[](past::integer_literal literal) -> fs::lang::object {
				return fs::lang::number{literal.value};
			},
			[](past::rarity_literal literal) -> fs::lang::object {
				return fs::lang::rarity{literal.value};
			},
			[](past::shape_literal literal) -> fs::lang::object {
				return fs::lang::shape{literal.value};
			},
			[](past::suit_literal literal) -> fs::lang::object {
				return fs::lang::suit{literal.value};
			},
			[](past::color_literal literal) -> fs::lang::object {
				return literal.to_lang_color();
			},
			[](past::string_literal literal) -> fs::lang::object {
				return fs::lang::string{literal.value};
			},
			[](past::identifier literal) -> fs::lang::object {
				assert(false);
				return fs::lang::boolean{false};
			}
		},
		literal);
}

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

}
