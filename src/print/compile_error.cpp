#include "print/compile_error.hpp"
#include "parser/state_handler.hpp"
#include <iostream>

namespace
{

void print_compile_error(fs::parser::state_handler& /* state */, fs::compiler::error::no_error /* error */)
{
	std::cout << "(no error)\n";
}

void print_compile_error(fs::parser::state_handler& state, fs::compiler::error::name_already_exists error)
{
	state.compile_error(error.place_of_error, "name already exists");
}

void print_compile_error(fs::parser::state_handler& state, fs::compiler::error::no_such_name error)
{
	state.compile_error(error.place_of_error, "no such name exists");
}

void print_compile_error(fs::parser::state_handler& /* state */, fs::compiler::error::type_mismatch /* error */)
{
	std::cout << "type mismatch\n";
}

void print_compile_error(fs::parser::state_handler& /* state */, fs::compiler::error::internal_error /* error */)
{
	std::cout << "internal compiler error\nplease report a bug with attached minimal source that reproduces it\n";
}

}

namespace fs::print
{

void compile_error(parser::state_handler& state, compiler::error::error_variant error)
{
	std::visit(
		[&state](auto error) { print_compile_error(state, error); },
		error);
}

}
