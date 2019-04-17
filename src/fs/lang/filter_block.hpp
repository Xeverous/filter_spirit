#pragma once
#include "fs/lang/condition_set.hpp"
#include "fs/lang/action_set.hpp"
#include <iosfwd>

namespace fs::lang
{

struct filter_block
{
	void generate(std::ostream& output_stream) const;

	bool show;
	condition_set conditions;
	action_set actions;
};

}
