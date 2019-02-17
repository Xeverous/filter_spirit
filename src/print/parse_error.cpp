#include "print/parse_error.hpp"
#include "print/generic.hpp"
#include "parser/config.hpp"

namespace fs::print
{

void print_parse_error(
	const parser::parse_error& error,
	const parser::lookup_data& lookup_data,
	std::ostream& error_stream)
{
	const iterator_type& error_first = error.error_place;
	const auto error_last = ++parser::iterator_type(error_first);
	print::print_line_number_with_indication_and_texts(
		error_stream,
		lookup_data.get_range_of_whole_content(),
		range_type(error_first, error_last),
		"parse error: expected '",
		error.what_was_expected,
		"' here");
}

}
