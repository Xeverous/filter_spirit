#include <fs/parser/parser.hpp>
#include <fs/parser/detail/config.hpp>
#include <fs/parser/detail/grammar.hpp>
#include <fs/log/logger.hpp>

#include <algorithm>

namespace fs::parser {
namespace {

template <
	typename ParsedFilterType,
	typename Ast,
	typename Grammar,
	typename Skipper
>
std::variant<ParsedFilterType, parse_failure_data>
parse_impl(
	std::string_view input,
	Grammar grammar,
	Skipper skipper)
{
	const iterator_type first{input.data()};
	const iterator_type last {input.data() + input.size()};
	detail::position_cache_type position_cache(first, last);
	// note: x3::with<> must match with grammar's context_type, otherwise you will get linker errors
	const auto parser = x3::with<detail::position_cache_tag>(std::ref(position_cache))[grammar];

	Ast ast;
	iterator_type it = first;
	bool result = false;
	parse_error error{first, {}};
	try {
		result = x3::phrase_parse(it, last, parser, skipper, ast);
	}
	catch (const x3::expectation_failure<iterator_type>& ex) {
		error.error_place = ex.where();
		error.what_was_expected = ex.which();
	}

	if (it != last || !result) {
		return parse_failure_data{
			parse_metadata{
				lookup_data(std::move(position_cache)),
				line_lookup(first, last)
			},
			std::move(error)
		};
	}

	return ParsedFilterType{
		std::move(ast),
		parse_metadata{
			lookup_data(std::move(position_cache)),
			line_lookup(first, last)
		},
	};
}

} // namespace

std::vector<range_type> line_lookup::split_lines(iterator_type first, iterator_type last)
{
	std::vector<range_type> result;
	// text length divided by average line length
	result.reserve(static_cast<std::size_t>((last - first) / 64));

	while (first != last) {
		const auto line_first = first;
		const auto line_last = utility::find_line_end(line_first, last);
		result.emplace_back(line_first, line_last);
		first = utility::skip_eol(line_last, last);
	}

	return result;
}

std::vector<range_type>::const_iterator line_lookup::find_line(iterator_type pos) const
{
	return std::lower_bound(_lines.begin(), _lines.end(), pos,
		[](range_type line, iterator_type pos) { return line.end() < pos; });
}

text_position line_lookup::text_position_for(iterator_type pos) const
{
	const auto it = find_line(pos);

	if (it == _lines.end())
		return text_position{_lines.size(), 0u};

	const auto line_number = static_cast<std::size_t>(it - _lines.begin());
	const auto column = static_cast<std::size_t>(pos - it->begin());
	return text_position{line_number, column};
}

text_context line_lookup::text_context_for(range_type range) const
{
	const auto it_first = find_line(range.begin());

	if (it_first == _lines.end())
		return text_context{_lines.size(), range_to_text(range)};

	const auto line_number = static_cast<std::size_t>(it_first - _lines.begin());
	const auto first = it_first->begin();

	const auto it_last = find_line(range.end());

	if (it_last == _lines.end())
		return text_context{line_number, std::string_view(first, range.size())};

	const auto last = it_last->end();

	return text_context{line_number, std::string_view(first, static_cast<std::size_t>(last - first))};
}

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
	if (parse_data.error.what_was_expected.empty()) {
		logger.error() << "Parse failed but no error information has been generated.\n" << log::strings::request_bug_report;
		return;
	}

	auto stream = logger.error();
	stream << "parse failure\n";

	text_context tc = parse_data.metadata.lines.text_context_for(range_type(parse_data.error.error_place, parse_data.error.error_place));
	stream.print_line_number_with_description(
		static_cast<int>(tc.line_number), "expected ", parse_data.error.what_was_expected, " here");
	stream.print_pointed_code(tc.surrounding_lines, parse_data.error.error_place);
}

}
