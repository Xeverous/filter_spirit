#include <fs/gui/windows/filter_windows.hpp>
#include <fs/gui/application.hpp>
#include <fs/utility/file.hpp>

#include <imgui.h>

namespace fs::gui {

void real_filter_window::draw_contents()
{
	ImGui::Text("so far nothing...");
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

	ImGui::Text(_status.c_str());

	_logger.draw();
}

void spirit_filter_window::reload_spirit_filter_file(const std::string& path)
{
	std::optional<std::string> file_content = fs::utility::load_file(path, _logger.logger());
	if (!file_content) {
		_state = std::nullopt;
		return;
	}

	_state.emplace(*file_content, _logger.logger());
	if (const auto& sf = (*_state).spirit_filter(); sf.has_value()) {
		_status = "filter loaded, ";
		_status += std::to_string((*sf).blocks.size());
		_status += " blocks";
	}
}

}
