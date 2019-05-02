#include "fs/parser/print_error.hpp"
#include "fs/parser/detail/config.hpp"
#include "fs/parser/detail/utility.hpp"
#include "fs/log/logger.hpp"

namespace
{

using iterator = fs::parser::detail::iterator_type;

[[nodiscard]]
iterator increment_if_not_last(iterator it, iterator last)
{
	if (it == last)
		return last;

	return ++it;
}

}

namespace fs::parser
{

void print_error(
	const parser::parse_error& error,
	const parser::lookup_data& lookup_data,
	log::logger&logger)
{
	const std::string_view content_range = lookup_data.get_view_of_whole_content();
	const detail::iterator_type& error_first = error.error_place;
	const detail::iterator_type error_last = increment_if_not_last(error_first, content_range.end());

	logger.begin_error_message();
	logger << "parse failure\n";
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		detail::to_string_view(detail::range_type(error_first, error_last)),
		"expected '",
		error.what_was_expected,
		"'");

	const detail::iterator_type parsed_place_first = error.parsed_place;
	const detail::iterator_type parsed_place_last = increment_if_not_last(parsed_place_first, content_range.end());
	logger.error_with_underlined_code(
		lookup_data.get_view_of_whole_content(),
		detail::to_string_view(detail::range_type(parsed_place_first, parsed_place_last)),
		"when parsing token starting here");

	logger.end_message();
}

}
