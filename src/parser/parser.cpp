#include "parser/parser.hpp"
#include "parser/grammar.hpp"
#include "print/structure_printer.hpp"

namespace fs::parser
{

std::optional<parse_result> parse(const std::string& file_content, std::ostream& error_stream)
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
		error_stream << "parse failure\n";
		return std::nullopt;
	}

	return parse_result(std::move(ast), std::move(position_cache));
}

void parse_result::print_ast() const
{
	print::structure_printer()(ast);
}

}
