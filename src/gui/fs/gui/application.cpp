#include <fs/gui/application.hpp>
#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/utility/file.hpp>
#include <fs/version.hpp>

#include <Magnum/GL/Context.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>

#include <initializer_list>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>

/**
 * code in this file - based on Magnum's ImGui example
 */

namespace {

using namespace fs;

void load_item_database(std::optional<lang::loot::item_database>& database, log::logger& logger)
{
	logger.info() << "loading item database...\n";

	std::optional<std::string> item_metadata_json = fs::utility::load_file("data/base_items.json", logger);
	if (!item_metadata_json) {
		logger.error() << "Failed to load item metadata, loot preview will be disabled.\n";
		return;
	}

	database.emplace();

	if (!(*database).parse(*item_metadata_json, logger)) {
		logger.error() << "Failed to parse item metadata, loot preview will be disabled.\n";
		database = std::nullopt;
		return;
	}

	logger.info() << "Item database loaded.";
}

}

namespace fs::gui {

application::application(const Arguments& arguments)
: Magnum::Platform::Application{
	arguments,
	Configuration{}
		.setTitle("Filter Spirit v" + to_string(fs::version::current()))
		.setWindowFlags(Configuration::WindowFlag::Resizable)}
, _application_log(font_settings())
, _color_picker(*this)
, _settings(*this)
{
	if (auto ctx = ImGui::CreateContext(); ctx == nullptr) {
		throw std::runtime_error("Failed to initialize Dear ImGui library!");
	}

	/*
	 * note: this has to be done in exactly this place:
	 * - after Dear ImGui's context has been constructed
	 * - before Magnum's Dear ImGui integration is constructed
	 * for more details, see documentation of Magnum::ImGuiIntegration::Context
	 */
	_application_log.logger().info() << "Loading and building fonts...\n";
	_fonting.build_default_fonts();
	_application_log.logger().info() << "Fonts ready.\n";

	_application_log.logger().info() << "Creating rendering context...\n";
	_imgui = Magnum::ImGuiIntegration::Context(
		*ImGui::GetCurrentContext(), Magnum::Vector2{windowSize()} / dpiScaling(), windowSize(), framebufferSize());
	_application_log.logger().info() << "Context created.\n";

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
	setMinimalLoopPeriod(_settings.min_frame_time_ms());
#endif

	load_item_database(_item_database, _application_log.logger());
	_leagues_cache.set_leagues(network::load_leagues_from_disk(_application_log.logger()));
	_price_report_cache.load_cache_file_from_disk(_application_log.logger());
	_theming.apply_current_theme();

	_application_log.logger().info() << "Application initialized.\n";
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
			if (ImGui::MenuItem("Open spirit filter template file...") && !_open_file_dialog.is_dialog_opened())
				_modal_dialog_state = modal_dialog_state_type::open_spirit_filter;
			if (ImGui::MenuItem("Open real filter file...") && !_open_file_dialog.is_dialog_opened())
				_modal_dialog_state = modal_dialog_state_type::open_real_filter;

			ImGui::Separator();

			if (ImGui::MenuItem("Spirit filter - from text input"))
				_spirit_filters.push_back(spirit_filter_window::from_text_input(*this));
			if (ImGui::MenuItem("Real filter - from text input"))
				_real_filters.push_back(real_filter_window::from_text_input(*this));

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
			if (ImGui::MenuItem(_color_picker.title().c_str())) {
				_color_picker.open();
				_color_picker.take_focus();
			}

#ifndef NDEBUG // right now only in debug builds, the thing is under construction
			if (ImGui::MenuItem(_single_item_preview.title().c_str())) {
				_single_item_preview.open();
				_single_item_preview.take_focus();
			}
#endif

			ImGui::Separator();

			ImGui::MenuItem("Fishing Cheatsheet", nullptr, false, false);
			aux::on_hover_text_tooltip("Complete Krillson's quests first in order to unlock this feature");

			ImGui::Separator();

			if (ImGui::MenuItem("Dear ImGui library demo window")) {
				_show_demo_window = true;
				_force_focus_demo_window = true;
			}
			aux::on_hover_text_tooltip(
				"This demo comes from one of FS dependencies.\n"
				"It is intended to showcase capabilities of Dear ImGui library.\n"
				"If you have an idea for a richer and/or different interface,\n"
				"this is the place to check what widgets are available.");

			ImGui::Separator();

			if (ImGui::MenuItem(_application_log.title().c_str())) {
				_application_log.open();
				_application_log.take_focus();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings")) {
			if (ImGui::MenuItem(_settings.title().c_str())) {
				_settings.open();
				_settings.take_focus();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About")) {
			if (ImGui::MenuItem(_about.title().c_str())) {
				_about.open();
				_about.take_focus();
			}

			if (ImGui::MenuItem(_version_info.title().c_str())) {
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
	tick_event();

	Magnum::GL::defaultFramebuffer.clear(Magnum::GL::FramebufferClear::Color);

	_imgui.newFrame();

	/* Enable text input, if needed */
	if (ImGui::GetIO().WantTextInput && !isTextInputActive())
		startTextInput();
	else if (!ImGui::GetIO().WantTextInput && isTextInputActive())
		stopTextInput();

	draw_main_menu_bar();

	_application_log.draw();

	for (auto& window : _spirit_filters)
		window.draw();
	for (auto& window : _real_filters)
		window.draw();

	_color_picker.draw();
	_single_item_preview.draw();
	_settings.draw();
	_about.draw();
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

void application::tick_event()
{
	remove_closed_windows();
	process_open_file_modals();
	rebuild_pending_fonts();
}

void application::remove_closed_windows()
{
	// remove dynamically created windows that have been closed
	// (with a pretty 1-statement STL erase-remove idiom)
	_spirit_filters.erase(
		std::remove_if(
			_spirit_filters.begin(),
			_spirit_filters.end(),
			[](const spirit_filter_window& w){ return !w.is_opened(); }),
		_spirit_filters.end());
	_real_filters.erase(
		std::remove_if(
			_real_filters.begin(),
			_real_filters.end(),
			[](const real_filter_window& w){ return !w.is_opened(); }),
		_real_filters.end());
}

void application::process_open_file_modals()
{
	if (!_open_file_dialog.is_dialog_opened()) {
		if (_modal_dialog_state == modal_dialog_state_type::open_spirit_filter) {
			_open_file_dialog.open_dialog("Select filter template file");
		}
		else if (_modal_dialog_state == modal_dialog_state_type::open_real_filter) {
			const auto patterns = { "*.filter" };
			_open_file_dialog.open_dialog(
				"Select real filter file",
				"Path of Exile Item Filters (*.filter)",
				patterns.begin(),
				patterns.end());
		}
	}

	if (_open_file_dialog.has_result()) {
		aux::file_dialog_result result = *_open_file_dialog.take_result();

		if (_modal_dialog_state == modal_dialog_state_type::open_spirit_filter) {
#ifdef __EMSCRIPTEN__
			if (result.file_name && result.file_content)
				_spirit_filters.push_back(spirit_filter_window::from_source(*this, *result.file_name, *result.file_content));
#else
			if (result.file_path)
				_spirit_filters.push_back(spirit_filter_window::from_file(*this, *result.file_path));
#endif
		}
		else if (_modal_dialog_state == modal_dialog_state_type::open_real_filter) {
#ifdef __EMSCRIPTEN__
			if (result.file_name && result.file_content)
				_real_filters.push_back(real_filter_window::from_source(*this, *result.file_name, *result.file_content));
#else
			if (result.file_path)
				_real_filters.push_back(real_filter_window::from_file(*this, *result.file_path));
#endif
		}

		_modal_dialog_state = modal_dialog_state_type::none;
	}
}

void application::rebuild_pending_fonts()
{
	// note: all of code below in this function has significant order requirements
	// otherwise the program crashes due to font/rendering dependency shenanigans
	// care when changing this function, for more - see implementation of fonting
	if (_fonting.is_rebuild_needed()) {
		_fonting.rebuild();
		_imgui.relayout(windowSize());
	}

	_fonting.update();
}

} // namespace fs::gui
