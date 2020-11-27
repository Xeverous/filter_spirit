#include <fs/lang/data_source_type.hpp>

namespace
{

constexpr auto poe_ninja_str = "poe.ninja";
constexpr auto poe_watch_str = "poe.watch";
constexpr auto none_str      = "(none)";

}

namespace fs::lang
{

const char* to_string(data_source_type data_source) noexcept
{
	if (data_source == data_source_type::poe_ninja)
		return poe_ninja_str;
	else if (data_source == data_source_type::poe_watch)
		return poe_watch_str;
	else if (data_source == data_source_type::none)
		return none_str;

	return "(unknown)";
}

std::optional<data_source_type> from_string(std::string_view data_source) noexcept
{
	if (data_source == poe_ninja_str)
		return data_source_type::poe_ninja;
	else if (data_source == poe_watch_str)
		return data_source_type::poe_watch;
	else if (data_source == none_str)
		return data_source_type::none;

	return std::nullopt;
}

}
