#include "fs/parser/print_error.hpp"
#include "fs/parser/config.hpp"
#include "fs/parser/utility.hpp"
#include "fs/log/logger.hpp"

namespace fs::parser
{

void print_error(
	const parser::parse_error& error,
	const parser::lookup_data& lookup_data,
	log::logger&logger)
{
	const std::string_view content_range = lookup_data.get_view_of_whole_content();
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
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		to_string_view(range_type(error_first, error_last)),
		"expected '",
		error.what_was_expected,
		"' here");
	logger.end_message();
}

}
