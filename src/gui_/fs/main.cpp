#include <fs/gui/application.hpp>

/*
 * This macro simply creates an object of specified type (giving {argc, argv} to ctor)
 * and calls .exec() on it.
 *
 * For Emscripten build, the macro instantiates the application instance as a global variable
 * instead of a local variable inside main(). This is in order to support the idle-efficient main
 * loop, as otherwise the local scope would end before any event callback has a chance to happen
 */
MAGNUM_APPLICATION_MAIN(fs::gui::application)
