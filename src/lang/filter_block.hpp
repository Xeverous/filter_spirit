#pragma once
#include "lang/condition_set.hpp"
#include "lang/action_set.hpp"

namespace fs::lang
{

struct filter_block
{
	bool show;
	condition_set conditions;
	action_set actions;
};

}
