#pragma once

#include <fs/lang/conditions.hpp>
#include <fs/lang/item_filter.hpp>
#include <fs/compiler/settings.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/compiler/detail/types.hpp>

#include <optional>

namespace fs::compiler::detail {

[[nodiscard]] std::optional<lang::autogen_extension>
make_autogen_extension(
	settings st,
	const lang::official_conditions& conditions,
	lang::price_range_condition price_range,
	autogen_protocondition autogen,
	lang::position_tag block_origin,
	diagnostics_store& diagnostics);

}
