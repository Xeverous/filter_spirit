#pragma once

#include <fs/lang/conditions.hpp>
#include <fs/lang/autogen_categories.hpp>

#include <optional>

namespace fs::compiler::detail {

// intermediate types for use inside the compiler implementation

struct autogen_protocondition
{
	lang::autogen_category category;
	lang::position_tag origin;
};

struct spirit_protoconditions
{
	lang::official_conditions official;

	// spirit filter extensions
	lang::price_range_condition price_range;
	std::optional<autogen_protocondition> autogen;
};

}
