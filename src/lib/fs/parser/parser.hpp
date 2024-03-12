#pragma once

#include <fs/parser/ast.hpp>
#include <fs/parser/detail/config.hpp>
#include <fs/log/logger.hpp>
#include <fs/utility/string_helpers.hpp>
#include <fs/utility/assert.hpp>

#include <string_view>
#include <variant>
#include <vector>
#include <utility>

namespace fs::parser
{

/*
 * All code related to text lines uses 0-based numbering.
 * First line (from user's perspective) is 0
 * First character in line (from user's perspective) is column 0
 */

struct text_position
{
	std::size_t line_number = 0;
	std::size_t pos = 0; // should be >= 0 and <= line.size()
};

struct text_range
{
	text_position first;
	text_position last;
};

struct text_context
{
	std::size_t line_number = 0;
	std::string_view surrounding_lines;
};

struct underlined_text
{
	text_context context;
	std::string_view underline;
};

inline std::string_view range_to_text(range_type range)
{
	return utility::make_string_view(range.begin(), range.end());
}

class line_lookup
{
public:
	line_lookup(iterator_type first, iterator_type last)
	: _lines(split_lines(first, last))
	{
	}

	text_context text_context_for(range_type range) const;

	text_position text_position_for(iterator_type pos) const;

	text_range text_range_for(range_type range) const
	{
		return text_range{
			text_position_for(range.begin()),
			text_position_for(range.end())
		};
	}

	std::string_view get_line(std::size_t n) const
	{
		if (n >= _lines.size())
			return {};

		return range_to_text(_lines[n]);
	}

	std::size_t num_lines() const
	{
		return _lines.size();
	}

private:
	static std::vector<range_type> split_lines(iterator_type first, iterator_type last);
	std::vector<range_type>::const_iterator find_line(iterator_type pos) const;

	std::vector<range_type> _lines;
};

class lookup_data
{
public:
	lookup_data(detail::position_cache_type position_cache)
	: _position_cache(std::move(position_cache))
	{
	}

	std::size_t size() const
	{
		return _position_cache.get_positions().size();
	}

	[[nodiscard]]
	std::string_view whole_text() const
	{
		return range_to_text(whole_range());
	}
	/**
	 * @note This wrapper functions have important aim: boost::position_cache::position_of() has 2
	 * overloads and will accept objects both of x3::position_tagged derived and non-derived types.
	 *
	 * For non-derived types it returns empty, invalid range - this is not wanted,
	 * empty ranges have no use in this program. Wrap the overload to force compiler
	 * error instead - much better than a segfault from unexpectedly invalid/empty range.
	 *
	 * The current position_cache_type used throws on invalid position_tagged input.
	 */
	[[nodiscard]]
	std::string_view text_of(const x3::position_tagged& ast) const
	{
		auto result = range_to_text(range_of(ast));
		// Some parsed ASTs will have EOL at the end - remove them.
		// Having EOLs at the end uglifies diagnostic messages.
		result = utility::rtrim(result, '\n');
		result = utility::rtrim(result, '\r');
		return result;
	}

private:
	[[nodiscard]]
	range_type range_of(const x3::position_tagged& ast) const
	{
		return _position_cache.position_of(ast);
	}

	[[nodiscard]]
	range_type whole_range() const
	{
		return range_type(_position_cache.first(), _position_cache.last());
	}

	detail::position_cache_type _position_cache;
};

struct parse_metadata
{
	underlined_text get_underlined_text(x3::position_tagged origin) const
	{
		std::string_view underline = lookup.text_of(origin);
		return underlined_text{lines.text_context_for(underline), underline};
	}

	lookup_data lookup;
	line_lookup lines;
};

struct parse_failure_data
{
	parse_metadata metadata;
	error_holder_type errors;
	iterator_type parser_stop_position;
};

void print_parse_errors(const parse_failure_data& parse_data, log::logger& logger);

struct parsed_spirit_filter
{
	ast::sf::ast_type ast;
	parse_metadata metadata;
};

[[nodiscard]]
std::variant<parsed_spirit_filter, parse_failure_data> parse_spirit_filter(std::string_view input);

struct parsed_real_filter
{
	ast::rf::ast_type ast;
	parse_metadata metadata;
};

[[nodiscard]]
std::variant<parsed_real_filter, parse_failure_data> parse_real_filter(std::string_view input);

} // namespace fs::parser
