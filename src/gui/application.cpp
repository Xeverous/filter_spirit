#include "application.hpp"

#include <boost/version.hpp>
#include <boost/config.hpp>

#include <fs/version.hpp>

#include <Magnum/GL/Context.h>
#include <Magnum/Version.h>
#include <Corrade/Version.h>

#include <initializer_list>
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
	setMinimalLoopPeriod(16); // 16ms per frame, roughly 60 FPS
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
			if (ImGui::MenuItem("Color picker"))
				_show_color_picker = true;
			if (ImGui::MenuItem("Dear ImGui library demo window"))
				_show_demo_window = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings")) {
			if (ImGui::MenuItem("Common UI settings"))
				_show_common_ui_settings = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About")) {
			if (ImGui::MenuItem("About FS"))
				_show_about_window = true;

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void button_click_to_copy(const char* text)
{
	if (ImGui::Button(text))
		ImGui::SetClipboardText(text);
}

void application::draw_color_picker()
{
	if (!ImGui::Begin("Color picker", &_show_color_picker)) {
		ImGui::End();
		return;
	}

	const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar;
	ImVec4 button_col = {
		_color_picker_selected_color.x(),
		_color_picker_selected_color.y(),
		_color_picker_selected_color.z(),
		_color_picker_selected_color.w()
	};
	ImGui::ColorButton("", button_col, flags, ImVec2(80, 80));
	ImGui::SameLine();
	ImGui::Text("click to copy\nthese strings");
	ImGui::SameLine();
	ImGui::BeginGroup();

	const auto rgba = Magnum::Math::pack<Magnum::Color4ub>(_color_picker_selected_color);
	const auto rgba_str = std::to_string(rgba.r()) + " " + std::to_string(rgba.g()) + " " + std::to_string(rgba.b()) + " " + std::to_string(rgba.a());
	button_click_to_copy(rgba_str.c_str());
	button_click_to_copy(("SetBorderColor " + rgba_str).c_str());
	button_click_to_copy(("SetTextColor " + rgba_str).c_str());
	button_click_to_copy(("SetBackgroundColor " + rgba_str).c_str());

	ImGui::EndGroup();
	ImGui::ColorPicker4("", _color_picker_selected_color.data(), flags);

	ImGui::End();
}

void application::draw_common_ui_settings()
{
	if (!ImGui::Begin("Common UI settings", &_show_common_ui_settings)) {
		ImGui::End();
		return;
	}

	if (ImGui::ColorEdit3("background color", _clear_color.data()))
		Magnum::GL::Renderer::setClearColor(_clear_color);

	if (ImGui::SliderInt("min frame time (ms)", &_frame_time_ms, 0, 40))
		setMinimalLoopPeriod(_frame_time_ms);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
		1000.0 / Magnum::Double(ImGui::GetIO().Framerate), Magnum::Double(ImGui::GetIO().Framerate));

	// TODO add font and scaling settings here

	ImGui::End();
}

void application::draw_about_window()
{
	constexpr auto line_width = 300;
	static const std::string fs_version = to_string(fs::version::current());
	static const std::string opengl_version = Magnum::GL::Context::current().versionString();
	static const std::string boost_version = std::to_string(BOOST_VERSION / 100000) + "." + std::to_string(BOOST_VERSION / 100 % 1000);
	static const std::string cpp_version = std::to_string(__cplusplus / 100);
	// __DATE__ has ugly "Mmm dd yyyy" format for legacy reasons, reorder it to ISO 8601
	const auto build_date = {
		__DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10], ' ',
		__DATE__[0], __DATE__[1], __DATE__[2], ' ',
		__DATE__[4], __DATE__[5], '\0'
	};

	if (!ImGui::Begin("About Filter Spirit", &_show_about_window)) {
		ImGui::End();
		return;
	}

	ImGui::Text("Version information of FS and its dependencies:");

	ImGui::Text("Filter Spirit version:");
	ImGui::SameLine(line_width);
	ImGui::Text(fs_version.c_str());

	ImGui::Text("Boost version:");
	ImGui::SameLine(line_width);
	ImGui::Text(boost_version.c_str());

	ImGui::Text("Magnum version:");
	ImGui::SameLine(line_width);
	ImGui::Text(MAGNUM_VERSION_STRING);

	ImGui::Text("Corrade version:");
	ImGui::SameLine(line_width);
	ImGui::Text(CORRADE_VERSION_STRING);

	ImGui::Text("Dear ImGui version:");
	ImGui::SameLine(line_width);
	ImGui::Text(IMGUI_VERSION);

	ImGui::Text("OpenGL version:");
	ImGui::SameLine(line_width);
	ImGui::Text(opengl_version.c_str());

	ImGui::Text("\n\n\n"); // TODO: find a better way for vertical space

	ImGui::Text("build information:");

	ImGui::Text("build date:");
	ImGui::SameLine(line_width);
	ImGui::Text(build_date.begin());

	ImGui::Text("C++ standard:");
	ImGui::SameLine(line_width);
	ImGui::Text(cpp_version.c_str());

	ImGui::Text("C++ compiler:");
	ImGui::SameLine(line_width);
	ImGui::Text(BOOST_COMPILER);

	ImGui::End();
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

	if (_show_color_picker)
		draw_color_picker();
	if (_show_demo_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow();
	}
	if (_show_common_ui_settings)
		draw_common_ui_settings();
	if (_show_about_window)
		draw_about_window();

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
