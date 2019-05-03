#include "fs/parser/parser.hpp"
#include "fs/parser/print_error.hpp"
#include "fs/parser/detail/grammar.hpp"
#include "fs/log/logger.hpp"
#include "fs/log/utility.hpp"

namespace fs::parser
{

std::variant<parse_success_data, parse_failure_data> parse(std::string_view input)
{
	const char *const first{input.data()};
	const char *const last {input.data() + input.size()};
	detail::position_cache_type position_cache(first, last);
	error_holder_type error_holder;
	// note: x3::with<> must match with grammar's context_type, otherwise you will get linker errors
	const auto parser = x3::with<detail::position_cache_tag>(std::ref(position_cache))
	[
		x3::with<detail::error_holder_tag>(std::ref(error_holder))[grammar()]
	];

	ast::ast_type ast;
	const char* it = first;
	const bool result = x3::phrase_parse(
		it,
		last,
		parser,
		skipper(),
		ast);

	if (it != last || !result)
		return parse_failure_data{lookup_data(std::move(position_cache)), std::move(error_holder), it};

	return parse_success_data{std::move(ast), lookup_data(std::move(position_cache))};
}

void print_parse_errors(const parse_failure_data& parse_data, log::logger& logger)
{
	if (parse_data.errors.empty())
	{
		logger.internal_error("parse failed but no error information has been reported");
		return;
	}

	for (const parse_error& error : parse_data.errors)
		print_error(error, parse_data.lookup_data, logger);

	logger.print_line_number_with_description_and_pointed_code(
		parse_data.lookup_data.get_view_of_whole_content(),
		parse_data.parser_stop_position,
		"parser stopped here");
}

}
