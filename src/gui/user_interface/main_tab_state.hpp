#pragma once

#include "event.hpp"
#include "user_interface/main_tab/gui_logger.hpp"
#include "user_interface/main_tab/builtin_options_state.hpp"
#include "user_interface/main_tab/filter_template_state.hpp"
#include "user_interface/main_tab/loot_preview_state.hpp"

#include <fs/log/logger.hpp>

#include <elements/element.hpp>

class main_tab_state
{
public:
	main_tab_state(event_inserter inserter)
	: _filter_template(inserter)
	, _builtin_options(inserter)
	, _loot_preview(inserter)
	, _root_element(make_ui())
	{
	}

	std::shared_ptr<cycfi::elements::element> ui()
	{
		return _root_element;
	}

	void update()
	{
		_logger.update();
		_builtin_options.update(_logger.logger());
	}

	auto& logger() { return _logger; }
	auto& filter_template() { return _filter_template; }
	auto& builtin_options() { return _builtin_options; }
	auto& loot_preview() { return _loot_preview; }

private:
	std::shared_ptr<cycfi::elements::element> make_ui();

	gui_logger _logger;
	filter_template_state _filter_template;
	builtin_options_state _builtin_options;
	loot_preview_state _loot_preview;

	std::shared_ptr<cycfi::elements::element> _root_element;
};
