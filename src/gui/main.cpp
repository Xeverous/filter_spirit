#include <elements/app.hpp>
#include <elements/view.hpp>
#include <elements/window.hpp>

#include <fs/version.hpp>

#include "ui.hpp"
#include "user_state.hpp"

namespace el = cycfi::elements;

int main(int argc, char* argv[])
{
	el::app app(argc, argv, "Filter Spirit", "xeverous.filter-spirit");
	el::window win("Filter Spirit v" + to_string(fs::version::current()));
	win.on_close = [&app]() { app.stop(); };

	user_state state;
	view v(win, state);
	make_and_bind_user_interface(win.host(), v, state);

	state.generation.league_state.refresh_available_leagues();

	app.run();

	return 0;
}
