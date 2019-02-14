#include "compiler/process_input.hpp"
#include "compiler/compiler.hpp"
#include "parser/parser.hpp"
// #include "print/structure_printer.hpp" // temporarily disabled

namespace fs::compiler
{

bool process_input(const std::string& file_content, std::ostream& error_stream)
{
	std::optional<std::pair<parser::ast::ast_type, parser::lookup_data>> parse_result = parser::parse(file_content, error_stream);

	if (!parse_result)
		return false;

	auto& ast = (*parse_result).first;
	auto& lookup_data = (*parse_result).second;

//	if (true) // allow easy switching on/off for now (before full implemenation of command line args)
//		print::structure_printer()(ast);

	if (!semantic_analysis(ast, lookup_data, error_stream))
		return false;

	return true;
}

}
