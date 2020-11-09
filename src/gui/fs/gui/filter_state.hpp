#pragma once

#include <fs/lang/item_filter.hpp>
#include <fs/log/logger.hpp>

#include <optional>
#include <string>
#include <utility>

namespace fs::gui {

class filter_template_state
{
public:
	filter_template_state() = default;
	filter_template_state(std::string source, fs::log::logger& logger)
	{
		new_source(std::move(source), logger);
	}

	bool new_source(std::string source, fs::log::logger& logger);
	bool parse_filter_template(fs::log::logger& logger);
	bool recompute_real_filter(fs::log::logger& logger);

	const auto& spirit_filter() const
	{
		return _spirit_filter;
	}

	const auto& real_filter() const
	{
		return _real_filter;
	}

private:
	std::optional<std::string> _template_source;
	std::optional<fs::lang::spirit_item_filter> _spirit_filter;
	std::optional<fs::lang::item_filter> _real_filter;
};

}
