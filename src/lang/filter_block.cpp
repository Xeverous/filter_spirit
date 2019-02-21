#include "lang/filter_block.hpp"
#include "lang/generation.hpp"
#include <ostream>

namespace fs::lang
{

void filter_block::generate(std::ostream& output_stream) const
{
	if (show)
		output_stream << generation::show;
	else
		output_stream << generation::hide;
	output_stream << '\n';

	conditions.generate(output_stream);
	actions.generate(output_stream);
}

}
