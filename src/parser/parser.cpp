#include "parser/parser.hpp"
#include "parser/grammar.hpp"
#include "print/generic.hpp"

namespace fs::parser
{

std::optional<std::pair<ast::ast_type, lookup_data>> parse(const std::string& file_content, std::ostream& error_stream)
{
	      iterator_type it   {file_content.begin()};
	const iterator_type begin{file_content.begin()};
	const iterator_type end  {file_content.end()};
	position_cache_type position_cache(begin, end);
	// note: x3::with<> must match with grammar's context_type, otherwise you will get linker errors
	const auto parser = x3::with<position_cache_tag>(std::ref(position_cache))
	[
		x3::with<error_stream_tag>(std::ref(error_stream))[fs::grammar()]
	];

	ast::ast_type ast;
	const bool result = x3::phrase_parse(
		it,
		end,
		parser,
		fs::skipper(),
		ast
	);

	if (it != end or !result) // fail if we did not get a full match
	{
		error_stream << "parse failure\nstopped at:\n";
		fs::print::print_line_with_indicator(error_stream, range_type(begin, end), range_type(it, ++iterator_type(it)));
	}

	return std::make_pair(std::move(ast), lookup_data(std::move(position_cache)));
}

}
