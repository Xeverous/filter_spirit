#include <fs/gui/application.hpp>
#include <fs/version.hpp>

#include <tinyfiledialogs.h>

#include <Magnum/GL/Context.h>

#include <initializer_list>
#include <algorithm>
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

void application::draw_main_menu_bar()
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open filter template file..."))
				_modal_dialog_state = modal_dialog_state_type::open_filter_template;
			if (ImGui::MenuItem("Open real filter file..."))
				_modal_dialog_state = modal_dialog_state_type::open_real_filter;

			ImGui::Separator();

#ifdef FILTER_SPIRIT_PLATFORM_WINDOWS
			if (ImGui::MenuItem("Exit", "Alt+F4"))
#else
			if (ImGui::MenuItem("Exit"))
#endif
			{
				exit();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools")) {
			if (ImGui::MenuItem(_color_picker.name().c_str())) {
				_color_picker.open();
				_color_picker.take_focus();
			}

			if (ImGui::MenuItem(_single_item_preview.name().c_str())) {
				_single_item_preview.open();
				_single_item_preview.take_focus();
			}

			if (ImGui::MenuItem("Dear ImGui library demo window")) {
				_show_demo_window = true;
				_force_focus_demo_window = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings")) {
			if (ImGui::MenuItem(_common_ui_settings.name().c_str())) {
				_common_ui_settings.open();
				_common_ui_settings.take_focus();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About")) {
			ImGui::MenuItem("About FS", nullptr, false, false);

			if (ImGui::MenuItem(_version_info.name().c_str())) {
				_version_info.open();
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

	for (auto& window : _filter_templates)
		window.draw();
	for (auto& window : _real_filters)
		window.draw();

	_color_picker.draw();
	_single_item_preview.draw();
	_common_ui_settings.draw();
	_version_info.draw();

	if (_show_demo_window) {
		if (_force_focus_demo_window) {
			ImGui::SetNextWindowFocus();
			_force_focus_demo_window = false;
		}

		ImGui::ShowDemoWindow(&_show_demo_window);
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

void application::tickEvent()
{
	// remove dynamically created windows that have been closed
	// (with a pretty 1-statement STL erase-remove idiom)
	_filter_templates.erase(
		std::remove_if(
			_filter_templates.begin(),
			_filter_templates.end(),
			[](const filter_template_window& w){ return !w.is_opened(); }),
		_filter_templates.end());
	_real_filters.erase(
		std::remove_if(
			_real_filters.begin(),
			_real_filters.end(),
			[](const real_filter_window& w){ return !w.is_opened(); }),
		_real_filters.end());

	if (_modal_dialog_state == modal_dialog_state_type::open_filter_template)
		on_open_filter_template();
	else if (_modal_dialog_state == modal_dialog_state_type::open_real_filter)
		on_open_real_filter();

	_modal_dialog_state = modal_dialog_state_type::none;
}

void application::on_open_filter_template()
{
	const char* const selected_path = tinyfd_openFileDialog(
		/* title */ "Select filter template file",
		/* default path */ nullptr,
		/* number of filter patterns */ 0,
		/* (no filters) */ nullptr,
		/* filter pattern description */ nullptr,
		/* multiselect allowed? */ 0);

	if (selected_path != nullptr)
		_filter_templates.emplace_back(selected_path);
}

void application::on_open_real_filter()
{
	auto patterns = { "*.filter" };

	const char* const selected_path = tinyfd_openFileDialog(
		/* title */ "Select real filter file",
		/* default path */ nullptr,
		/* number of filter patterns */ patterns.size(),
		/* filters array */ patterns.begin(),
		/* filter pattern description */ "Path of Exile Item Filters",
		/* multiselect allowed? */ 0);

	if (selected_path != nullptr)
		_real_filters.emplace_back(selected_path);
}

} // namespace fs::gui