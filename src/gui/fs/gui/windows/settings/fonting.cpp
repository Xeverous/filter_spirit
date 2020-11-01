#include <fs/gui/windows/settings/fonting.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/file.hpp>

#include <imgui.h>

#include <utility>

namespace {

fs::gui::font load_font(std::filesystem::path path, std::string name)
{
	std::error_code ec;
	fs::gui::font result;
	result.font_data = fs::utility::load_file(path, ec);
	if (ec)
		throw std::system_error(ec, "failed to load " + path.generic_string());
	result.name = std::move(name);
	return result;
}

}

namespace fs::gui {

fonting::fonting()
{
	_text_fonts.push_back(load_font("assets/fonts/Fontin-Regular.otf", "Fontin Regular"));
	_text_fonts.push_back(load_font("assets/fonts/Fontin-SmallCaps.otf", "Fontin SmallCaps"));
}

void fonting::build_default_fonts()
{
	FS_ASSERT(_text_fonts.size() == 2u);
	_selected_text_font_index = 1;
	rebuild();
}

void fonting::draw_font_selection_ui()
{
	if (ImGui::BeginCombo("Application font", selected_text_font_name().c_str())) {
		for (std::size_t i = 0; i < _text_fonts.size(); ++i) {
			font& fnt = _text_fonts[i];
			ImGui::PushID(&fnt);
			if (ImGui::Selectable(fnt.name.c_str(), i == _selected_text_font_index)) {
				_selected_text_font_index = i;
				_rebuild_needed = true;
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	if (ImGui::SliderInt("Application font size (px)", &_text_font_size, 1, 48))
		_rebuild_needed = true;
}

void fonting::rebuild()
{
	ImGuiIO& io = ImGui::GetIO();
	FS_ASSERT(io.Fonts != nullptr);
	io.Fonts->Clear();

	ImFontConfig config;
	config.FontDataOwnedByAtlas = false;

	FS_ASSERT(_selected_text_font_index < _text_fonts.size());
	font& fnt = _text_fonts[_selected_text_font_index];
	_text_font = io.Fonts->AddFontFromMemoryTTF(const_cast<char*>(fnt.font_data.c_str()), fnt.font_data.size(), _text_font_size, &config);
	FS_ASSERT(_text_font != nullptr);
	io.FontDefault = _text_font;

	_rebuild_needed = false;
}

}
