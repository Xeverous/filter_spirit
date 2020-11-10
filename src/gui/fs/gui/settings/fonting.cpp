#include <fs/gui/settings/fonting.hpp>
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

fonting::fonting()
{
#define FONTS_PATH "assets/fonts"
	_text_fonts.emplace_back(FONTS_PATH "/fontin/Fontin-Regular.otf",   "Fontin Regular");
	_text_fonts.emplace_back(FONTS_PATH "/fontin/Fontin-SmallCaps.otf", "Fontin SmallCaps");

	_monospaced_fonts.emplace_back(FONTS_PATH "/monoid/Monoid-Regular-NoCalt.ttf", "Monoid Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/monoid/Monoid-Bold-NoCalt.ttf",    "Monoid Bold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/monoid/Monoid-Italic-NoCalt.ttf",  "Monoid Italic");
	_monospaced_fonts.emplace_back(FONTS_PATH "/monoid/Monoid-Retina-NoCalt.ttf",  "Monoid Retina");

	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Light.ttf",    "Fira Code Light");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Regular.ttf",  "Fira Code Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Medium.ttf",   "Fira Code Medium");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-SemiBold.ttf", "Fira Code SemiBold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Bold.ttf",     "Fira Code Bold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/fira_code/ttf/FiraCode-Retina.ttf",   "Fira Code Retina");

	_monospaced_fonts.emplace_back(FONTS_PATH "/inconsolata/Inconsolata.otf", "Inconsolata");

	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-ExtraLight.ttf", "Jet Brains Mono ExtraLight");
	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-Light.ttf",      "Jet Brains Mono Light");
	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-SemiLight.ttf",  "Jet Brains Mono SemiLight");
	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-Regular.ttf",    "Jet Brains Mono Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-Medium.ttf",     "Jet Brains Mono Medium");
	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-Bold.ttf",       "Jet Brains Mono Bold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/jet_brains_mono/ttf/JetBrainsMono-ExtraBold.ttf",  "Jet Brains Mono ExtraBold");

	_monospaced_fonts.emplace_back(FONTS_PATH "/hack/Hack-Regular.ttf",    "Hack Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/hack/Hack-Bold.ttf",       "Hack Bold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/hack/Hack-BoldItalic.ttf", "Hack Bold Italic");
	_monospaced_fonts.emplace_back(FONTS_PATH "/hack/Hack-Italic.ttf",     "Hack Italic");

	_monospaced_fonts.emplace_back(FONTS_PATH "/roboto_mono/static/RobotoMono-Thin.ttf",       "Roboto Mono Thin");
	_monospaced_fonts.emplace_back(FONTS_PATH "/roboto_mono/static/RobotoMono-Bold.ttf",       "Roboto Mono Bold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/roboto_mono/static/RobotoMono-ExtraLight.ttf", "Roboto Mono ExtraLight");
	_monospaced_fonts.emplace_back(FONTS_PATH "/roboto_mono/static/RobotoMono-Light.ttf",      "Roboto Mono Light");
	_monospaced_fonts.emplace_back(FONTS_PATH "/roboto_mono/static/RobotoMono-Medium.ttf",     "Roboto Mono Medium");
	_monospaced_fonts.emplace_back(FONTS_PATH "/roboto_mono/static/RobotoMono-Regular.ttf",    "Roboto Mono Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/roboto_mono/static/RobotoMono-SemiBold.ttf",   "Roboto Mono SemiBold");

	_monospaced_fonts.emplace_back(FONTS_PATH "/ubuntu_mono/UbuntuMono-Regular.ttf",    "Ubuntu Mono Regular");
	_monospaced_fonts.emplace_back(FONTS_PATH "/ubuntu_mono/UbuntuMono-Bold.ttf",       "Ubuntu Mono Bold");
	_monospaced_fonts.emplace_back(FONTS_PATH "/ubuntu_mono/UbuntuMono-BoldItalic.ttf", "Ubuntu Mono Bold Italic");
	_monospaced_fonts.emplace_back(FONTS_PATH "/ubuntu_mono/UbuntuMono-Italic.ttf",     "Ubuntu Mono Italic");
#undef FONTS_PATH
}

void fonting::build_default_fonts()
{
	FS_ASSERT(_text_fonts.size() >= 2u);
	_selected_text_font_index = 1;
	FS_ASSERT(_text_fonts.size() >= 11u);
	_selected_monospaced_font_index = 10;
	rebuild();
}

void fonting::draw_font_selection_ui()
{
	draw_font_selection_ui_impl("Application font", "Application font size (px)",
		_text_fonts, _selected_text_font_index, _text_font_size, _rebuild_needed);
	draw_font_selection_ui_impl("Monospaced font", "Monospaced font size (px)",
		_monospaced_fonts, _selected_monospaced_font_index, _monospaced_font_size, _rebuild_needed);

	if (ImGui::Checkbox("Use monospaced font in application interface", &_monospaced_interface_font))
		_update_needed = true;
}

void fonting::update()
{
	if (!_update_needed)
		return;

	if (ImGuiIO& io = ImGui::GetIO(); _monospaced_interface_font)
		io.FontDefault = _monospaced_font;
	else
		io.FontDefault = _text_font;

	_update_needed = false;
}

void fonting::rebuild()
{
	ImGuiIO& io = ImGui::GetIO();
	FS_ASSERT(io.Fonts != nullptr);
	ImFontAtlas& atlas = *io.Fonts;
	atlas.Clear();

	FS_ASSERT(_selected_text_font_index < _text_fonts.size());
	_text_font = build_font(atlas, _text_fonts[_selected_text_font_index], _text_font_size);
	FS_ASSERT(_selected_monospaced_font_index < _monospaced_fonts.size());
	_monospaced_font = build_font(atlas, _monospaced_fonts[_selected_monospaced_font_index], _monospaced_font_size);

	_rebuild_needed = false;
	_update_needed = true;
}

}
