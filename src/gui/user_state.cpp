#include "user_state.hpp"

#include <fs/generator/generate_filter.hpp>
#include <fs/utility/file.hpp>

void user_state::load_filter_template()
{
	filter.template_source = fs::utility::load_file(generation.filter_template_path, logger);
	if (filter.template_source)
		logger.info() << "loaded filter template, " << (*filter.template_source).size() << " bytes";
}

void user_state::parse_filter_template()
{
	if (!filter.template_source)
		return;

	auto& source = *filter.template_source;
	filter.spirit_filter = fs::generator::sf::parse_spirit_filter(source, {}, logger);
}
