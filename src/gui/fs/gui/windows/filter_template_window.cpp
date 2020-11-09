#include <fs/gui/application.hpp>
#include <fs/gui/windows/filter_template_window.hpp>
#include <fs/utility/file.hpp>

#include <imgui.h>

namespace fs::gui {

filter_template_window::filter_template_window(application& app, std::string path)
: imgui_window(std::move(path))
, _application(std::ref(app))
, _logger(app.font_settings())
{
	open();
	reload_filter_template_file(filter_template_path());
}

void filter_template_window::draw_contents()
{
	if (ImGui::Button("reload filter template file"))
		reload_filter_template_file(filter_template_path());

	ImGui::Text(_status.c_str());

	_logger.draw();
}

void filter_template_window::reload_filter_template_file(const std::string& path)
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
