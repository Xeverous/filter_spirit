#pragma once
#include "print/generic.hpp"
#include "compiler/error.hpp"

namespace fs::parser
{
	class state_handler;
}

namespace fs::print
{

// TODO remove dependency on state_handler
void compile_error(parser::state_handler& state, compiler::error::error_variant error);

}
