#include <fs/lang/data_source_type.hpp>

namespace
{

constexpr auto poe_ninja_str = "poe.ninja";
constexpr auto poe_watch_str = "poe.watch";

}

namespace fs::lang
{

std::string_view to_string(data_source_type data_source) noexcept
{
	if (data_source == data_source_type::poe_ninja)
		return poe_ninja_str;
	else if (data_source == data_source_type::poe_watch)
		return poe_watch_str;

	return "(unknown)";
}

std::optional<data_source_type> from_string(std::string_view data_source) noexcept
{
	if (data_source == poe_ninja_str)
		return data_source_type::poe_ninja;
	else if (data_source == poe_watch_str)
		return data_source_type::poe_watch;

	return std::nullopt;
}

}
