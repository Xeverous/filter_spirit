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
	const iterator_type& error_first = error.error_place;
	const auto error_last = ++iterator_type(error_first);
	logger.begin_error_message();
	logger << "parse failure\n";
	print::print_line_number_with_indication_and_texts(
		logger,
		lookup_data.get_range_of_whole_content(),
		range_type(error_first, error_last),
		"expected '",
		error.what_was_expected,
		"' here");
	logger.end_message();
}

}
