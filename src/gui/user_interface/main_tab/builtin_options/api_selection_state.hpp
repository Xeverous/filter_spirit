#pragma once

#include "event.hpp"

#include <fs/lang/data_source_type.hpp>

#include <elements/element/element.hpp>

#include <memory>

class api_selection_state
{
public:
	api_selection_state(event_inserter inserter)
	: _inserter(inserter)
	{
	}

	fs::lang::data_source_type selected_api() const
	{
		return _selected_api;
	}

	std::shared_ptr<cycfi::elements::element> make_ui();

private:
	event_inserter _inserter;
	fs::lang::data_source_type _selected_api = fs::lang::data_source_type::poe_ninja;
};
