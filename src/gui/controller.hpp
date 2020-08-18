#pragma once

#include "user_interface.hpp"
#include "event.hpp"

#include <elements/view.hpp>
#include <elements/window.hpp>

#include <queue>

class controller : public cycfi::elements::view
{
public:
	controller(cycfi::elements::window& w)
	: cycfi::elements::view(w)
	, _ui(*this, event_inserter(_event_queue))
	{
	}

	// hook extra code into view::poll in order to call update once per frame
	void poll() override
	{
		cycfi::elements::view::poll();
		_ui.update();
		process_events();
		layout();
	}

private:
	void process_events();

	std::queue<event_t> _event_queue;
	user_interface _ui;
};
