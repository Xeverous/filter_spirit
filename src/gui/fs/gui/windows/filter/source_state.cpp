#include <fs/gui/windows/filter/filter_state_mediator_base.hpp>
#include <fs/gui/windows/filter/source_state.hpp>
#include <fs/utility/file.hpp>

#include <imgui.h>

#include <utility>

namespace fs::gui {

void source_state::draw_interface(filter_state_mediator_base& mediator)
{
	if (ImGui::CollapsingHeader("Source", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::Button("Reload"))
			reload_source_file(mediator);

		ImGui::SameLine();

		if (_source)
			ImGui::TextWrapped("Source present, %zu bytes", (*_source).size());
		else
			ImGui::TextWrapped("Source not present.");
	}
}

void source_state::load_source_file(std::string path, filter_state_mediator_base& mediator)
{
	_file_path = std::move(path);
	reload_source_file(mediator);
}

void source_state::reload_source_file(filter_state_mediator_base& mediator)
{
	if (_file_path)
		new_source(utility::load_file(*_file_path, mediator.logger()), mediator);
}

void source_state::new_source(std::optional<std::string> source, filter_state_mediator_base& mediator)
{
	_source = std::move(source);
	mediator.on_source_change(_source);
}

}
