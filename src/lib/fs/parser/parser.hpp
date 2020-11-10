#pragma once

#include <fs/parser/ast.hpp>
#include <fs/parser/detail/config.hpp>
#include <fs/log/logger.hpp>
#include <fs/utility/string_helpers.hpp>
#include <fs/utility/assert.hpp>

#include <string_view>
#include <variant>
#include <utility>

namespace fs::parser
{

class lookup_data
{
public:
	lookup_data(detail::position_cache_type position_cache)
	: position_cache(std::move(position_cache))
	{
	}

	[[nodiscard]]
	std::string_view get_view_of_whole_content() const
	{
		const detail::range_type range = get_range_of_whole_content();
		return utility::make_string_view(range.begin(), range.end());
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
	std::string_view position_of(const x3::position_tagged& ast) const
	{
		const detail::range_type range = range_of(ast);
		return utility::make_string_view(range.begin(), range.end());
	}

private:
	[[nodiscard]]
	detail::range_type range_of(const x3::position_tagged& ast) const
	{
		return position_cache.position_of(ast);
	}

	[[nodiscard]]
	detail::range_type get_range_of_whole_content() const
	{
		FS_ASSERT(!position_cache.get_positions().empty());
		return detail::range_type(position_cache.first(), position_cache.last());
	}

	detail::position_cache_type position_cache;
};

struct parse_failure_data
{
	lookup_data lookup;
	error_holder_type errors;
	const char* parser_stop_position; // not a C string but a string iterator
};

void print_parse_errors(const parse_failure_data& parse_data, log::logger& logger);

struct parsed_spirit_filter
{
	ast::sf::ast_type ast;
	lookup_data lookup;
};

[[nodiscard]]
std::variant<parsed_spirit_filter, parse_failure_data> parse_spirit_filter(std::string_view input);

struct parsed_real_filter
{
	ast::rf::ast_type ast;
	lookup_data lookup;
};

[[nodiscard]]
std::variant<parsed_real_filter, parse_failure_data> parse_real_filter(std::string_view input);

} // namespace fs::parser
