#include <fs/parser/parser.hpp>
#include <fs/parser/detail/grammar.hpp>
#include <fs/log/logger.hpp>

namespace
{

namespace parser = fs::parser;
namespace x3 = boost::spirit::x3;

template <
	typename ParsedFilterType,
	typename Ast,
	typename Grammar,
	typename Skipper
>
std::variant<ParsedFilterType, parser::parse_failure_data>
parse_impl(
	std::string_view input,
	Grammar grammar,
	Skipper skipper)
{
	const char *const first{input.data()};
	const char *const last {input.data() + input.size()};
	parser::detail::position_cache_type position_cache(first, last);
	parser::error_holder_type error_holder;
	// note: x3::with<> must match with grammar's context_type, otherwise you will get linker errors
	const auto parser = x3::with<parser::detail::position_cache_tag>(std::ref(position_cache))
	[
		x3::with<parser::detail::error_holder_tag>(std::ref(error_holder))[grammar]
	];

	Ast ast;
	const char* it = first;
	const bool result = x3::phrase_parse(
		it,
		last,
		parser,
		skipper,
		ast);

	if (it != last || !result) {
		return parser::parse_failure_data{
			parser::lookup_data(std::move(position_cache)),
			std::move(error_holder),
			it
		};
	}

	return ParsedFilterType{std::move(ast), parser::lookup_data(std::move(position_cache))};
}

void print_error(
	const parser::parse_error& error,
	const parser::lookup_data& lookup_data,
	fs::log::logger& logger)
{
	auto stream = logger.error();
	stream << "parse failure\n";
	stream.print_line_number_with_description_and_pointed_code(
		lookup_data.get_view_of_whole_content(),
		error.error_place,
		"expected ",
		error.what_was_expected,
		" here");
	stream.print_line_number_with_description_and_pointed_code(
		lookup_data.get_view_of_whole_content(),
		error.backtracking_place,
		"backtracking parser here");
}

} // namespace

namespace fs::parser
{

std::variant<parsed_spirit_filter, parse_failure_data> parse_spirit_filter(std::string_view input)
{
	return parse_impl<parsed_spirit_filter, ast::sf::ast_type>(input, detail::sf_grammar(), detail::sf_skipper());
}

std::variant<parsed_real_filter, parse_failure_data> parse_real_filter(std::string_view input)
{
	return parse_impl<parsed_real_filter, ast::rf::ast_type>(input, detail::rf_grammar(), detail::rf_skipper());
}

void print_parse_errors(const parse_failure_data& parse_data, log::logger& logger)
{
	if (parse_data.errors.empty()) {
		logger.error() << "Parse failed but no error information has been generated.\n" << log::strings::request_bug_report;
		return;
	}

	for (const parse_error& error : parse_data.errors)
		print_error(error, parse_data.lookup, logger);

	logger.info().print_line_number_with_description_and_pointed_code(
		parse_data.lookup.get_view_of_whole_content(),
		parse_data.parser_stop_position,
		"parser stopped here");
}

}
