#include <fs/gui/settings/font_settings.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/file.hpp>

#include <imgui.h>

#include <utility>
#include <system_error>

namespace {

void draw_font_selection_ui_impl(
	const char* desc_font,
	const char* desc_font_size,
	const std::vector<fs::gui::font>& fonts,
	std::size_t& selected_font_index,
	int& font_size,
	bool& rebuild_needed)
{
	FS_ASSERT(selected_font_index < fonts.size());

	if (ImGui::BeginCombo(desc_font, fonts[selected_font_index].name().c_str())) {
		for (std::size_t i = 0; i < fonts.size(); ++i) {
			const fs::gui::font& fnt = fonts[i];
			ImGui::PushID(&fnt);
			if (ImGui::Selectable(fnt.name().c_str(), i == selected_font_index)) {
				selected_font_index = i;
				rebuild_needed = true;
			}
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	constexpr auto min_font_size = 1;
	constexpr auto max_font_size = 48;
	if (ImGui::SliderInt(desc_font_size, &font_size, min_font_size, max_font_size))
		rebuild_needed = true;
}

// returns non-owning pointer
ImFont* build_font(ImFontAtlas& atlas, fs::gui::font& fnt, int font_size_px)
{
	ImFontConfig config;
	config.FontDataOwnedByAtlas = false;
	ImFont* result = atlas.AddFontFromMemoryTTF(const_cast<char*>(fnt.data().c_str()), fnt.data().size(), font_size_px, &config);
	FS_ASSERT(result != nullptr);
	return result;
}

}

namespace fs::gui {

void font::load()
{
	std::error_code ec;
	_font_data = fs::utility::load_file(_path, ec);
	if (ec)
		throw std::system_error(ec, "failed to load " + _path);
}

font_settings::font_settings()
{
#define FONTS_PATH "assets/fonts"
	_text_fonts.emplace_back(FONTS_PATH "/fontin/Fontin-SmallCaps.otf", "Fontin SmallCaps");
	_text_fonts.emplace_back(FONTS_PATH "/fontin/Fontin-Regular.otf",   "Fontin Regular");

	_monospaced_fonts.emplace_back(FONTS_PATH "/ubuntu_mono/UbuntuMono-Regular.ttf", "Ubuntu Mono Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/ubuntu_mono/UbuntuMono-Bold.ttf",    "Ubuntu Mono Bold");

	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Light.ttf",    "Fira Code Light");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Regular.ttf",  "Fira Code Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Medium.ttf",   "Fira Code Medium");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-SemiBold.ttf", "Fira Code SemiBold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Bold.ttf",     "Fira Code Bold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Retina.ttf",   "Fira Code Retina");

	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-Regular.ttf",   "Jet Brains Mono Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-Medium.ttf",    "Jet Brains Mono Medium");
	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-Bold.ttf",      "Jet Brains Mono Bold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-ExtraBold.ttf", "Jet Brains Mono ExtraBold");

	_monospaced_fonts.emplace_back(FONTS_PATH "/hack/Hack-Regular.ttf", "Hack Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/hack/Hack-Bold.ttf",    "Hack Bold");
#undef FONTS_PATH
}

void font_settings::build_default_fonts()
{
	FS_ASSERT(!_text_fonts.empty());
	FS_ASSERT(!_monospaced_fonts.empty());
	rebuild();
}

void font_settings::draw_font_selection_ui()
{
	draw_font_selection_ui_impl("Application font", "Application font size (px)",
		_text_fonts, _selected_text_font_index, _text_font_size, _rebuild_needed);
	draw_font_selection_ui_impl("Monospaced font", "Monospaced font size (px)",
		_monospaced_fonts, _selected_monospaced_font_index, _monospaced_font_size, _rebuild_needed);

	if (ImGui::Checkbox("Use monospaced font in application interface", &_monospaced_interface_font))
		_update_needed = true;
}

ImFont* font_settings::filter_preview_font(int size) const
{
	FS_ASSERT(size >= lang::constants::min_filter_font_size);
	FS_ASSERT(size <= lang::constants::max_filter_font_size);
	const std::size_t index = size - lang::constants::min_filter_font_size;
	FS_ASSERT(index < _filter_preview_fonts.size());
	const auto result = _filter_preview_fonts[index];
	FS_ASSERT(result != nullptr);
	return result;
}

void font_settings::update()
{
	if (!_update_needed)
		return;

	if (ImGuiIO& io = ImGui::GetIO(); _monospaced_interface_font)
		io.FontDefault = _monospaced_font;
	else
		io.FontDefault = _text_font;

	_update_needed = false;
}

void font_settings::rebuild()
{
	ImGuiIO& io = ImGui::GetIO();
	FS_ASSERT(io.Fonts != nullptr);
	ImFontAtlas& atlas = *io.Fonts;
	atlas.Clear();

	FS_ASSERT(_selected_text_font_index < _text_fonts.size());
	_text_font = build_font(atlas, _text_fonts[_selected_text_font_index], _text_font_size);
	FS_ASSERT(_text_font != nullptr);
	FS_ASSERT(_selected_monospaced_font_index < _monospaced_fonts.size());
	_monospaced_font = build_font(atlas, _monospaced_fonts[_selected_monospaced_font_index], _monospaced_font_size);
	FS_ASSERT(_monospaced_font != nullptr);

	FS_ASSERT(!_text_fonts.empty()); // loop below assumes that preview font is first in _text_fonts
	for (std::size_t i = 0; i < _filter_preview_fonts.size(); ++i) {
		_filter_preview_fonts[i] = build_font(atlas, _text_fonts.front(), i + lang::constants::min_filter_font_size);
		FS_ASSERT(_filter_preview_fonts[i] != nullptr);
	}

	_rebuild_needed = false;
	_update_needed = true;
}

}
