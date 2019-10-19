#include <fs/parser/print_error.hpp>
#include <fs/parser/detail/config.hpp>
#include <fs/log/logger.hpp>

namespace fs::parser
{

void print_error(
	const parser::parse_error& error,
	const parser::lookup_data& lookup_data,
	log::logger&logger)
{
	logger.begin_error_message();
	logger << "parse failure\n";
	logger.print_line_number_with_description_and_pointed_code(
		lookup_data.get_view_of_whole_content(),
		error.error_place,
		"expected ",
		error.what_was_expected,
		" here");
	logger.print_line_number_with_description_and_pointed_code(
		lookup_data.get_view_of_whole_content(),
		error.backtracking_place,
		"backtracking parser here");
	logger.end_message();
}

}
