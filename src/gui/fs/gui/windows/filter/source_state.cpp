#include <fs/gui/windows/filter/filter_state_mediator_base.hpp>
#include <fs/gui/windows/filter/source_state.hpp>
#include <fs/gui/settings/fonting.hpp>
#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/utility/file.hpp>
#include <fs/utility/assert.hpp>

#include <imgui.h>

#include <utility>

constexpr auto popup_text_input_title = "Filter source - text edit";

namespace fs::gui {

void source_state::draw_interface(const fonting& f, filter_state_mediator_base& mediator)
{
	if (ImGui::CollapsingHeader("Source", ImGuiTreeNodeFlags_DefaultOpen)) {
#ifndef __EMSCRIPTEN__
		if (is_from_file()) {
			if (ImGui::Button("Reload"))
				reload_source_file(mediator);

			ImGui::SameLine();
		}
#endif

		if (ImGui::Button("Edit filter source"))
			open_source_edit();

		aux::on_hover_text_tooltip("Any changes here are temporary and do not change the file.");

		ImGui::SameLine();

		if (_source)
			ImGui::TextWrapped("Source present, %zu bytes", (*_source).size());
		else
			ImGui::TextWrapped("Source not present.");
	}

	/*
	 * The boolean flag is necessary to delay invokation of opening popup
	 * because Dear ImGui expects begin/open/close calls at same levels
	 * of the ID stack.
	 */
	if (_popup_pending) {
		if (_source)
			_text_input.set_text(*_source);

		_popup_pending = false;
		ImGui::OpenPopup(popup_text_input_title);
	}

	if (ImGui::BeginPopupModal(popup_text_input_title)) {
		if (aux::button_negative("Cancel"))
			on_text_input_cancel();

		ImGui::SameLine();

		if (aux::button_positive("Done"))
			on_text_input_done(mediator);

		ImGui::SameLine();

		if (ImGui::Button("Paste from clipboard")) {
			_text_input.set_text(ImGui::GetClipboardText());
		}

		{
			auto _ = f.scoped_monospaced_font();
			_text_input.draw({-1.0f, -1.0f}); // fill remaining space
		}

		ImGui::EndPopup();
	}
}

void source_state::open_source_edit()
{
	_popup_pending = true;
}

void source_state::on_text_input_done(filter_state_mediator_base& mediator)
{
	_is_source_edited = true;

	FS_ASSERT(ImGui::IsPopupOpen(popup_text_input_title));
	ImGui::CloseCurrentPopup();

	if (_source)
		(*_source) = _text_input.c_str();
	else
		_source.emplace(_text_input.c_str());

	_file_path = std::nullopt;

	mediator.on_source_change(source());
}

void source_state::on_text_input_cancel()
{
	FS_ASSERT(ImGui::IsPopupOpen(popup_text_input_title));
	ImGui::CloseCurrentPopup();
}

#ifndef __EMSCRIPTEN__
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
#endif

void source_state::new_source(std::optional<std::string> src, filter_state_mediator_base& mediator)
{
	_source = std::move(src);
	_is_source_edited = false;
	mediator.on_source_change(source());
}

}
