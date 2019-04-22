#include "fs/print/parse_error.hpp"
#include "fs/print/generic.hpp"
#include "fs/parser/config.hpp"
#include "fs/log/logger.hpp"

namespace fs::print
{

void print_parse_error(
	const parser::parse_error& error,
	const parser::lookup_data& lookup_data,
	logger& logger)
{
	const range_type content_range = lookup_data.get_range_of_whole_content();
	const iterator_type& error_first = error.error_place;
	// make the error range at least 1 position unless error_first already points at end
	const iterator_type error_last = [&]()
	{
		if (error_first == content_range.end())
			return error_first;

		iterator_type error_last = error_first;
		return ++error_last;
	}();

	logger.begin_error_message();
	logger << "parse failure\n";
	print::print_line_number_with_indication_and_texts(
		logger,
		content_range,
		range_type(error_first, error_last),
		"expected '",
		error.what_was_expected,
		"' here");
	logger.end_message();
}

}
