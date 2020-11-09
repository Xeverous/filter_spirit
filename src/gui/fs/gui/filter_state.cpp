#include <fs/gui/filter_state.hpp>
#include <fs/generator/generate_filter.hpp>
#include <fs/generator/make_item_filter.hpp>

#include <utility>

namespace fs::gui {

bool filter_template_state::new_source(std::string source, fs::log::logger& logger)
{
	_template_source = std::move(source);
	return parse_filter_template(logger);
}

bool filter_template_state::parse_filter_template(fs::log::logger& logger)
{
	if (!_template_source)
		return false;

	_spirit_filter = fs::generator::sf::parse_spirit_filter(*_template_source, {}, logger);
	return recompute_real_filter(logger);
}

bool filter_template_state::recompute_real_filter(fs::log::logger& /* logger */)
{
	if (!_spirit_filter)
		return false;

	_real_filter = fs::generator::make_item_filter(*_spirit_filter, {});
	return _real_filter.has_value();
}

}
