#include "user_state.hpp"

#include <fs/generator/generate_filter.hpp>
#include <fs/generator/make_filter.hpp>
#include <fs/network/poe_ninja/download_data.hpp>
#include <fs/utility/file.hpp>
#include <fs/log/monitor.hpp>

void user_state::load_filter_template()
{
	filter.template_source = fs::utility::load_file(generation.filter_template_path, logger.monitor());
	if (filter.template_source) {
		logger.monitor()([this](fs::log::logger& logger) {
			logger.info() << "loaded filter template, " << (*filter.template_source).size() << " bytes";
		});
	}
}

void user_state::parse_filter_template()
{
	if (!filter.template_source)
		return;

	auto& source = *filter.template_source;
	logger.monitor()([this, &source](fs::log::logger& logger) {
		filter.spirit_filter = fs::generator::sf::parse_spirit_filter(source, {}, logger);
	});

	recompute_real_filter();
}

void user_state::recompute_real_filter()
{
	if (!filter.spirit_filter)
		return;

	filter.real_filter = fs::generator::make_filter(*filter.spirit_filter, generation.item_price_data_state.price_report.data);
}

void user_state::update()
{
	generation.league_state.update();
	generation.item_price_data_state.update();
	logger.update();
}
