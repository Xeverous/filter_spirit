#pragma once

#include "user_state.hpp"

#include <elements/view.hpp>
#include <elements/window.hpp>

void
make_and_bind_user_interface(
	cycfi::elements::host_window_handle window,
	cycfi::elements::view& v,
	user_state& state);
