#pragma once

#include "user_interface/main_tab_state.hpp"
#include "user_interface/settings_tab_state.hpp"
#include "event.hpp"

#include <elements/view.hpp>
#include <elements/window.hpp>

class user_interface
{
public:
	user_interface(cycfi::elements::host_window_handle window, cycfi::elements::view& v, event_inserter inserter);

	void update()
	{
		_main_tab.update();
	}

	auto& main() { return _main_tab; }
	auto& settings() { return _settings_tab; }

private:
	main_tab_state _main_tab;
	settings_tab_state _settings_tab;
};
