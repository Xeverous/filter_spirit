#include <fs/gui/application.hpp>
#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/gui/windows/filter_windows_fwd.hpp>
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

void load_item_database(lang::loot::item_database& database, log::logger& logger)
{
	logger.info() << "loading item database...\n";

	constexpr auto str_error_msg = "Failed to load item data, loot generation will not produce any items.\n";

	std::optional<std::string> item_data_json = fs::utility::load_file("data/base_items.json", logger);
	if (!item_data_json) {
		logger.error() << str_error_msg;
		return;
	}

	if (!database.parse(*item_data_json, logger)) {
		logger.error() << str_error_msg;
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
	_settings.settings().fonting.build_default_fonts();
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
	setMinimalLoopPeriod(_settings.settings().min_frame_time_ms);
#endif

	load_item_database(_item_database, _application_log.logger());
	_network_cache.leagues.set_leagues(network::load_leagues_from_disk(_application_log.logger()));
	_network_cache.item_price_reports.load_cache_file_from_disk(_application_log.logger());
	_settings.settings().theming.apply_current_theme();

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

			if (ImGui::MenuItem(str_spirit_filter_from_text_input))
				_filters.push_back(spirit_filter_window_from_text_input(_network_cache));
			if (ImGui::MenuItem(str_real_filter_from_text_input))
				_filters.push_back(real_filter_window_from_text_input());

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

	_application_log.draw(_settings.settings().fonting);

	for (auto& window : _filters)
		window->draw(_settings.settings(), _item_database, _loot_generator, _network_cache);

	_color_picker.draw(_settings.settings().fonting);
	_single_item_preview.draw();
	_settings.draw(*this);
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
	_filters.erase(
		std::remove_if(
			_filters.begin(),
			_filters.end(),
			[](const std::unique_ptr<filter_window>& ptr){ return !ptr->is_opened(); }),
		_filters.end());
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
				_filters.push_back(spirit_filter_window_from_source(_network_cache, *result.file_name, *result.file_content));
#else
			if (result.file_path)
				_filters.push_back(spirit_filter_window_from_file(_network_cache, *result.file_path));
#endif
		}
		else if (_modal_dialog_state == modal_dialog_state_type::open_real_filter) {
#ifdef __EMSCRIPTEN__
			if (result.file_name && result.file_content)
				_filters.push_back(real_filter_window_from_source(*result.file_name, *result.file_content));
#else
			if (result.file_path)
				_filters.push_back(real_filter_window_from_file(*result.file_path));
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
	if (_settings.settings().fonting.is_rebuild_needed()) {
		_settings.settings().fonting.rebuild();
		_imgui.relayout(windowSize());
	}

	_settings.settings().fonting.update();
}

} // namespace fs::gui
