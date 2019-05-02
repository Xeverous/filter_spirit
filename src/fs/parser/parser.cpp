#include "fs/parser/parser.hpp"
#include "fs/parser/print_error.hpp"
#include "fs/parser/detail/grammar.hpp"
#include "fs/log/logger.hpp"

namespace
{

namespace pr = fs::parser;
namespace x3 = boost::spirit::x3;

std::pair<pr::lookup_data, std::variant<pr::ast::ast_type, pr::detail::error_holder_type>> parse_impl(std::string_view input)
{
	      pr::detail::iterator_type it   {input.begin()};
	const pr::detail::iterator_type begin{input.begin()};
	const pr::detail::iterator_type end  {input.end()};
	pr::detail::position_cache_type position_cache(begin, end);
	pr::detail::error_holder_type error_holder;
	// note: x3::with<> must match with grammar's context_type, otherwise you will get linker errors
	const auto parser = x3::with<pr::detail::position_cache_tag>(std::ref(position_cache))
	[
		x3::with<pr::detail::error_holder_tag>(std::ref(error_holder))[pr::grammar()]
	];

	pr::ast::ast_type ast;
	const bool result = x3::phrase_parse(
		it,
		end,
		parser,
		pr::skipper(),
		ast);

	using result_variant = std::variant<pr::ast::ast_type, pr::detail::error_holder_type>;

	if (it != end || !result)
	{
		// TODO
		//error_stream << "parse failure\nstopped at:\n";
		//fs::print::print_line_with_indicator(error_stream, range_type(begin, end), range_type(it, ++iterator_type(it)));
		return std::make_pair(pr::lookup_data(std::move(position_cache)), result_variant(std::move(error_holder)));
	}

	return std::make_pair(pr::lookup_data(std::move(position_cache)), result_variant(std::move(ast)));
}

}

namespace fs::parser
{

std::optional<parse_data> parse(std::string_view input, log::logger& logger)
{
	std::pair<lookup_data, std::variant<ast::ast_type, detail::error_holder_type>> parse_result = parse_impl(input);

	auto& lookup_data = parse_result.first;
	auto& ast_or_error = parse_result.second;

	if (std::holds_alternative<detail::error_holder_type>(ast_or_error))
	{
		auto& errors = std::get<detail::error_holder_type>(ast_or_error);

		if (errors.empty())
		{
			logger.internal_error("parse failed but no error information has been reported");
			return std::nullopt;
		}

		for (const parse_error& error : errors)
			print_error(error, lookup_data, logger);

		return std::nullopt;
	}

	auto& ast = std::get<ast::ast_type>(ast_or_error);
	return parse_data{std::move(ast), std::move(lookup_data)};
}

}
