#pragma once

#include <fs/lang/conditions.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/detail/types.hpp>

namespace fs::compiler::detail {

[[nodiscard]] std::function<lang::blocks_generator_func_type> // empty on failure
make_autogen_func(
	settings st,
	const lang::official_conditions& conditions,
	lang::price_range_condition price_range,
	autogen_protocondition autogen,
	lang::position_tag block_origin,
	diagnostics_store& diagnostics);

}
