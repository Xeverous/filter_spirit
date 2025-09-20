#pragma once

#include <fs/lang/conditions.hpp>
#include <fs/lang/enum_types.hpp>

#include <boost/container/small_vector.hpp>

#include <optional>

namespace fs::compiler::detail {

// intermediate types for use inside the compiler implementation

struct autogen_protocondition
{
	using autogen_container_type = boost::container::small_vector<lang::autogen_category, 1u>;

	autogen_container_type categories;
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
