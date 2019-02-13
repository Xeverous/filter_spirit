#include "lang/condition_set.hpp"

namespace fs::lang
{

std::variant<condition_set, compiler::error::error_variant> condition_set::add_restrictions(condition_set other) const
{
	// FIXME: implement
	return other;
}

}
