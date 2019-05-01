#pragma once

#include "fs/parser/ast.hpp"
#include "fs/parser/config.hpp"
#include "fs/parser/utility.hpp"

#include <string_view>
#include <optional>
#include <utility>
#include <cassert>

namespace fs::log { class logger; }

namespace fs::parser
{

class lookup_data
{
public:
	lookup_data(position_cache_type position_cache)
	: position_cache(std::move(position_cache))
	{
	}

	[[nodiscard]]
	std::string_view get_view_of_whole_content() const
	{
		return to_string_view(get_range_of_whole_content());
	}
	/**
	 * @note This wrapper functions have important aim: boost::position_cache::position_of() has 2
	 * overloads and will accept objects both of x3::position_tagged derived and non-derived types.
	 *
	 * For non-derived types it returns empty, invalid range - this is not wanted,
	 * empty ranges have no use in this program. Wrap the overload and force compiler
	 * errror instead - much better than a segfault from unexpectedly invalid/empty range.
	 */
	[[nodiscard]]
	std::string_view position_of(const x3::position_tagged& ast) const
	{
		return to_string_view(range_of(ast));
	}

private:
	[[nodiscard]]
	range_type range_of(const x3::position_tagged& ast) const
	{
		return position_cache.position_of(ast);
	}

	[[nodiscard]]
	range_type get_range_of_whole_content() const
	{
		assert(!position_cache.get_positions().empty());
		return range_type(position_cache.first(), position_cache.last());
	}

	position_cache_type position_cache;
};

struct parse_data
{
	ast::ast_type ast;
	lookup_data lookup_data;
};

[[nodiscard]]
std::optional<parse_data> parse(std::string_view input, log::logger& logger);

}
