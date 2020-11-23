#include <fs/gui/windows/filter/filter_state_mediator_base.hpp>
#include <fs/gui/windows/filter/source_state.hpp>
#include <fs/utility/file.hpp>

#include <imgui.h>

#include <utility>

constexpr auto popup_text_input_title = "Filter source - text edit";

namespace fs::gui {

void source_state::draw_interface(filter_state_mediator_base& mediator)
{
	if (ImGui::CollapsingHeader("Source", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (is_from_file()) {
			if (ImGui::Button("Reload"))
				reload_source_file(mediator);

			ImGui::SameLine();
		}

		if (ImGui::Button("From file...")) {

		}

		ImGui::SameLine();

		if (ImGui::Button("From text input")) {
			ImGui::OpenPopup(popup_text_input_title);
		}

		ImGui::SameLine();

		if (_source)
			ImGui::TextWrapped("Source present, %zu bytes", (*_source).size());
		else
			ImGui::TextWrapped("Source not present.");
	}

	if (ImGui::BeginPopupModal(popup_text_input_title)) {
		if (ImGui::Button("Done")) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
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
