#include <elements/app.hpp>
#include <elements/view.hpp>
#include <elements/window.hpp>

#include <fs/version.hpp>

#include "ui.hpp"

namespace el = cycfi::elements;

int main(int argc, char* argv[])
{
	el::app app(argc, argv);
	el::window win("Filter Spirit v" + fs::version::version_string());
	win.on_close = [&app]() { app.stop(); };

	el::view view(win);
	user_state state;
	make_and_bind_user_interface(win.host(), view, state);

	app.run();

	return 0;
}
