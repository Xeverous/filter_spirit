#include "controller.hpp"

#include <fs/version.hpp>

#include <elements/app.hpp>
#include <elements/view.hpp>
#include <elements/window.hpp>

namespace el = cycfi::elements;

int main(int argc, char* argv[])
{
	el::app app(argc, argv, "Filter Spirit", "com.xeverous.filter-spirit");
	el::window win("Filter Spirit v" + to_string(fs::version::current()));
	win.on_close = [&app]() { app.stop(); };

	controller c(win);
	app.run();

	return 0;
}
