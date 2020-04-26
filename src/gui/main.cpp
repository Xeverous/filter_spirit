#include <elements/app.hpp>
#include <elements/window.hpp>
#include <elements/support.hpp>
#include <elements/element.hpp>

using namespace cycfi::elements;

// Main window background color
auto constexpr bkd_color = rgba(35, 35, 37, 255);
auto background = box(bkd_color);

int main(int argc, const char* argv[])
{
   app _app(argc, argv);
   window _win(_app.name());
   _win.on_close = [&_app]() { _app.stop(); };

   view view_(_win);

   view_.content(
                     // Add more content layers here. The order
                     // specifies the layering. The lowest layer
                     // is at the bottom of this list.

      background     // Replace background with your main element,
                     // or keep it and add another layer on top of it.
   );

   _app.run();
   return 0;
}
