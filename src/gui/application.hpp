#include <Magnum/Math/Color.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/ImGuiIntegration/Context.hpp>

#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#include <Magnum/Platform/EmscriptenApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

namespace fs::gui {

using namespace Magnum::Math::Literals;

class application: public Magnum::Platform::Application
{
public:
	explicit application(const Arguments& arguments);

	void drawEvent() override;

	void viewportEvent(ViewportEvent& event) override;

	void keyPressEvent(KeyEvent& event) override;
	void keyReleaseEvent(KeyEvent& event) override;

	void mousePressEvent(MouseEvent& event) override;
	void mouseReleaseEvent(MouseEvent& event) override;
	void mouseMoveEvent(MouseMoveEvent& event) override;
	void mouseScrollEvent(MouseScrollEvent& event) override;
	void textInputEvent(TextInputEvent& event) override;

private:
	void draw_main_menu_bar();
	void draw_color_picker();
	void draw_common_ui_settings();
	void draw_about_window();

	Magnum::ImGuiIntegration::Context _imgui{Magnum::NoCreate};

	ImFont* _fontin_regular = nullptr;
	ImFont* _fontin_small_caps = nullptr;

	bool _show_color_picker = false;
	Magnum::Color4 _color_picker_selected_color = 0xffffffff_rgbaf;

	bool _show_demo_window = false;
	bool _show_common_ui_settings = false;
	bool _show_about_window = false;
	Magnum::Color4 _clear_color = 0x72909aff_rgbaf;
	int _frame_time_ms = 16;
};

}
