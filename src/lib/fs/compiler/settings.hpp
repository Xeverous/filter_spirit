#pragma once

#include <fs/lang/style_overrides.hpp>
#include <fs/lang/enum_types.hpp>

namespace fs::compiler
{

struct error_handling_settings
{
	bool stop_on_error = false;
	bool treat_warnings_as_errors = false;
};

struct settings
{
	bool is_ruthless() const noexcept { return game_variant == lang::game_variant_type::poe1_ruthless; }
	bool is_poe1() const noexcept { return !is_poe2(); }
	bool is_poe2() const noexcept { return game_variant == lang::game_variant_type::poe2; }

	lang::game_variant_type game_variant =  lang::game_variant_type::poe1;
	bool print_ast = false;
	error_handling_settings error_handling;
	lang::style_overrides overrides;
};

}
