#pragma once

#include "fs/lang/filter_block.hpp"
#include "fs/generator/metadata.hpp"

#include <string>
#include <vector>

namespace fs::generator
{

[[nodiscard]]
std::string assemble_blocks_to_raw_filter(const std::vector<lang::filter_block>& blocks);

void prepend_metadata(metadata metadata, std::string& raw_filter);

}
