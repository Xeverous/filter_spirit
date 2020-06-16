#pragma once

#include "user_state.hpp"

#include <elements/view.hpp>
#include <elements/window.hpp>

class view : public cycfi::elements::view
{
public:
	view(cycfi::elements::window& w, user_state& state)
	: cycfi::elements::view(w), _state(state)
	{
	}

	// hook extra code into view's poll in order to call update once per frame
	void poll() override
	{
		cycfi::elements::view::poll();
		_state.update();
		layout();
	}

private:
	user_state& _state;
};

void
make_and_bind_user_interface(
	cycfi::elements::host_window_handle window,
	view& v,
	user_state& state);
