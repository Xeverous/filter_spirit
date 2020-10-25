#include "application.hpp"

#include <Magnum/GL/Context.h>

#include <fs/version.hpp>

#include <exception>
#include <stdexcept>

/**
 * code in this file - based on Magnum's ImGui example
 */

namespace {

// returns non-owning pointer
ImFont* load_font_from_file(ImGuiIO& io, const char* path, float size_pixels)
{
	ImFont* result = io.Fonts->AddFontFromFileTTF(path, size_pixels);
	if (result == nullptr)
		throw std::runtime_error(std::string("can not load font: ") + path);
	return result;
}

}

namespace fs::gui {

application::application(const Arguments& arguments)
: Magnum::Platform::Application{
	arguments,
	Configuration{}
		.setTitle("Filter Spirit v" + to_string(fs::version::current()))
		.setWindowFlags(Configuration::WindowFlag::Resizable)}
, _common_ui_settings(*this)
{
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	_fontin_regular    = load_font_from_file(io, "assets/fonts/Fontin-Regular.otf", 24);
	_fontin_small_caps = load_font_from_file(io, "assets/fonts/Fontin-SmallCaps.otf", 24);
	io.FontDefault = _fontin_small_caps;

	_imgui = Magnum::ImGuiIntegration::Context(
		*ImGui::GetCurrentContext(), Magnum::Vector2{windowSize()} / dpiScaling(), windowSize(), framebufferSize());

	/* Set up proper blending to be used by ImGui. There's a great chance
	   you'll need this exact behavior for the rest of your scene. If not, set
	   this only for the drawFrame() call. */
	Magnum::GL::Renderer::setBlendEquation(
		Magnum::GL::Renderer::BlendEquation::Add,
		Magnum::GL::Renderer::BlendEquation::Add);
	Magnum::GL::Renderer::setBlendFunction(
		Magnum::GL::Renderer::BlendFunction::SourceAlpha,
		Magnum::GL::Renderer::BlendFunction::OneMinusSourceAlpha);

#if !defined(MAGNUM_TARGET_WEBGL) && !defined(CORRADE_TARGET_ANDROID)
	setMinimalLoopPeriod(_common_ui_settings.min_frame_time_ms());
#endif
}

void application::draw_main_menu_bar()
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("Open filter template file...");
			ImGui::MenuItem("Open real filter file...");

			ImGui::Separator();

			if (ImGui::MenuItem("Exit", "Alt+F4"))
				exit();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools")) {
			if (ImGui::MenuItem(_color_picker.name())) {
				_color_picker.show();
				_color_picker.take_focus();
			}

			if (ImGui::MenuItem(_single_item_preview.name())) {
				_single_item_preview.show();
				_single_item_preview.take_focus();
			}

			if (ImGui::MenuItem("Dear ImGui library demo window")) {
				_show_demo_window = true;
				_force_focus_demo_window = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings")) {
			if (ImGui::MenuItem(_common_ui_settings.name())) {
				_common_ui_settings.show();
				_common_ui_settings.take_focus();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About")) {
			ImGui::MenuItem("About FS", nullptr, false, false);

			if (ImGui::MenuItem(_version_info.name())) {
				_version_info.show();
				_version_info.take_focus();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void application::drawEvent()
{
	Magnum::GL::defaultFramebuffer.clear(Magnum::GL::FramebufferClear::Color);

	_imgui.newFrame();

	/* Enable text input, if needed */
	if (ImGui::GetIO().WantTextInput && !isTextInputActive())
		startTextInput();
	else if (!ImGui::GetIO().WantTextInput && isTextInputActive())
		stopTextInput();

	draw_main_menu_bar();

	_color_picker.draw();
	_single_item_preview.draw();
	_common_ui_settings.draw();
	_version_info.draw();

	if (_show_demo_window) {
		if (_force_focus_demo_window) {
			ImGui::SetNextWindowFocus();
			_force_focus_demo_window = false;
		}

		ImGui::ShowDemoWindow();
	}

	/* Update application cursor */
	_imgui.updateApplicationCursor(*this);

	/* Set appropriate states. If you only draw ImGui, it is sufficient to
	   just enable blending and scissor test in the constructor. */
	Magnum::GL::Renderer::enable(Magnum::GL::Renderer::Feature::Blending);
	Magnum::GL::Renderer::enable(Magnum::GL::Renderer::Feature::ScissorTest);
	Magnum::GL::Renderer::disable(Magnum::GL::Renderer::Feature::FaceCulling);
	Magnum::GL::Renderer::disable(Magnum::GL::Renderer::Feature::DepthTest);

	_imgui.drawFrame();

	/* Reset state. Only needed if you want to draw something else with
	   different state after. */
	Magnum::GL::Renderer::enable(Magnum::GL::Renderer::Feature::DepthTest);
	Magnum::GL::Renderer::enable(Magnum::GL::Renderer::Feature::FaceCulling);
	Magnum::GL::Renderer::disable(Magnum::GL::Renderer::Feature::ScissorTest);
	Magnum::GL::Renderer::disable(Magnum::GL::Renderer::Feature::Blending);

	swapBuffers();
	redraw();
}

void application::viewportEvent(ViewportEvent& event)
{
	Magnum::GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});
	_imgui.relayout(Magnum::Vector2{event.windowSize()} / event.dpiScaling(), event.windowSize(), event.framebufferSize());
}

void application::keyPressEvent(KeyEvent& event)
{
	if (_imgui.handleKeyPressEvent(event))
		return;
}

void application::keyReleaseEvent(KeyEvent& event)
{
	if (_imgui.handleKeyReleaseEvent(event))
		return;
}

void application::mousePressEvent(MouseEvent& event)
{
	if (_imgui.handleMousePressEvent(event))
		return;
}

void application::mouseReleaseEvent(MouseEvent& event)
{
	if (_imgui.handleMouseReleaseEvent(event))
		return;
}

void application::mouseMoveEvent(MouseMoveEvent& event)
{
	if (_imgui.handleMouseMoveEvent(event))
		return;
}

void application::mouseScrollEvent(MouseScrollEvent& event)
{
	if (_imgui.handleMouseScrollEvent(event)) {
		/* Prevent scrolling the page */
		event.setAccepted();
		return;
	}
}

void application::textInputEvent(TextInputEvent& event)
{
	if (_imgui.handleTextInputEvent(event))
		return;
}

} // namespace fs::gui
