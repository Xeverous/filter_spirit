#include "parser/state_handler.hpp"
#include "parser/ast_adapted.hpp"
#include "print/structure_printer.hpp"
#include <iostream>

namespace fs::parser
{

void state_handler::print_ast() const
{
	print::structure_printer()(ast);
}

void state_handler::print_map() const
{
	for (const auto& pair : map)
	{
		std::cout << pair.first << "\n";
	}
}

}

