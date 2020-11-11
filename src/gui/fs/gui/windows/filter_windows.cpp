#include <fs/gui/windows/filter_windows.hpp>
#include <fs/gui/windows/filter/filter_state.hpp>
#include <fs/gui/application.hpp>
#include <fs/utility/file.hpp>

#include <imgui.h>

namespace {

using namespace fs;
using namespace fs::gui;

void reload_filter_helper(const std::string& path, gui_logger& logger, std::string& status, detail::filter_state_base& state)
{
	std::optional<std::string> file_content = utility::load_file(path, logger.logger());
	if (!file_content) {
		status = "loading failed";
		state.new_source(std::nullopt, logger.logger());
		return;
	}

	status = "filter loaded, ";

	state.new_source(std::move(file_content), logger.logger());

	if (const auto& rep = state.filter_representation(); rep.has_value()) {
		status += std::to_string((*rep).blocks.size());
		status += " blocks";
	}
	else {
		status += "parsing/compiling failed";
	}
}

} // namespace

namespace fs::gui {

real_filter_window::real_filter_window(application& app, std::string path)
: imgui_window(std::move(path))
, _application(std::ref(app))
, _logger(app.font_settings())
{
	open();
	reload_real_filter_file(real_filter_path());
}

void real_filter_window::draw_contents()
{
	if (ImGui::Button("reload real filter file"))
		reload_real_filter_file(real_filter_path());

	ImGui::SameLine();
	ImGui::TextUnformatted(_status.c_str());

	_logger.draw();

	if (_state.filter_representation()) {
		// draw debug functionality
	}
}

void real_filter_window::reload_real_filter_file(const std::string& path)
{
	reload_filter_helper(path, _logger, _status, _state);
}

spirit_filter_window::spirit_filter_window(application& app, std::string path)
: imgui_window(std::move(path))
, _application(std::ref(app))
, _logger(app.font_settings())
{
	open();
	reload_spirit_filter_file(spirit_filter_path());
}

void spirit_filter_window::draw_contents()
{
	if (ImGui::Button("reload spirit filter template file"))
		reload_spirit_filter_file(spirit_filter_path());

	ImGui::SameLine();
	ImGui::TextUnformatted(_status.c_str());

	_logger.draw();

	if (_state.filter_representation()) {
		// draw debug functionality
	}
}

void spirit_filter_window::reload_spirit_filter_file(const std::string& path)
{
	reload_filter_helper(path, _logger, _status, _state);
}

}
