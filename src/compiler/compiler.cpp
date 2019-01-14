#include "compiler/compiler.hpp"
#include "compiler/ast.hpp"
#include "compiler/error.hpp"
#include "parser/parser.hpp"
#include "utility/holds_alternative.hpp"
#include <unordered_map>
#include <cassert>

namespace
{

	fs::compiler::error_type add_constant(
		const fs::parser::ast::constant_definition& def,
		fs::compiler::constants_map& map)
	{
		const auto is_there_name = [&map](const std::string& name) {
			return map.find(name) != map.end();
		};

		namespace past = fs::parser::ast;

		if (is_there_name(def.name.value))
			return fs::compiler::name_already_exists();

		if (holds_alternative<past::identifier>(def.value.get()))
		{
			const auto it = map.find(boost::get<past::identifier>(def.value.get()).value);
			if (it == map.end())
				return fs::compiler::no_such_name();

			const fs::compiler::ast::object_type& object = *it;
			// FIXME: check that object's type matches def.object_type
			map.emplace(def.name, object);
			return fs::compiler::no_error();
		}

		// from this point on, assume that the value expression does not store an identifier
		// so if the expression type does not match expected type we can surely return an error

		switch (def.object_type.value)
		{
			using ott = fs::parser::ast::object_type_type;

			case ott::boolean:
			{
				if (holds_alternative<past::boolean_literal>(def.value.get()))
					map.emplace(def.name.value, boost::get<past::boolean_literal>(def.value.get()).value);
				else
					return fs::compiler::type_mismatch();

				break;
			}
			case ott::number:
			{
				if (holds_alternative<past::integer_literal>(def.value.get()))
					map.emplace(def.name.value, boost::get<past::integer_literal>(def.value.get()).value);
				else
					return fs::compiler::type_mismatch();

				break;
			}
			default:
			{
				assert(false);
				break;
			}
		}

		return fs::compiler::no_error();
	}
}

namespace fs::compiler
{



std::variant<constants_map, error_type> parse_constants(const parser::ast::ast_type& ast)
{
	constants_map map;

	for (const parser::ast::code_line& line : ast)
	{
		if (line.value)
		{
			const error_type error = add_constant(*line.value, map);
			if (!std::holds_alternative<no_error>(error))
				return error;
		}
	}

	return map;
}

}
