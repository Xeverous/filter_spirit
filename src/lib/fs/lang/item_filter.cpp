#include <fs/lang/item_filter.hpp>
#include <fs/lang/keywords.hpp>

#include <ostream>

namespace fs::lang
{

void item_filter_block::generate(std::ostream& output_stream) const
{
	if (!conditions.is_valid())
		return;

	if (visibility.show)
		output_stream << keywords::rf::show;
	else
		output_stream << keywords::rf::hide;
	output_stream << '\n';

	conditions.generate(output_stream);
	actions.generate(output_stream);
	output_stream << '\n';
}

}
