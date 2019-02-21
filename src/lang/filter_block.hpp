#pragma once
#include "lang/condition_set.hpp"
#include "lang/action_set.hpp"
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
