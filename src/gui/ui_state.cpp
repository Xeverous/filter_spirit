#include "ui_state.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <type_traits>

// header uses unsigned to avoid SDL/OpenGL include pollution
// verify here that it is the same type
static_assert(std::is_same_v<GLuint, unsigned>);

namespace fs::gui {

namespace {

void nk_sdl_device_upload_atlas(GLuint& font_tex, const void* image, GLsizei width, GLsizei height)
{
	glBindTexture(GL_TEXTURE_2D, font_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
}

}

void ui_state::regenerate_font_atlas()
{
	if (font_size <= 0.0f)
		return;

	atlas.begin();

	nk::font_config font_config(font_size);
	font_config.get().oversample_h = 1;
	font_config.get().pixel_snap = 1;

	/* Load Cursor: if you add cursor loading please hide the cursor */

	// add fonts here... if none are loaded a default font will be used
	fontin_regular = atlas.add_from_file("assets/fonts/fontin/Fontin-Regular.otf", font_config);
	NUKLEUS_ASSERT(fontin_regular != nullptr);
	fontin_small_caps = atlas.add_from_file("assets/fonts/fontin/Fontin-SmallCaps.otf", font_config);
	NUKLEUS_ASSERT(fontin_small_caps != nullptr);
	monospace = atlas.add_from_file("assets/fonts/ubuntu_mono/UbuntuMono-Regular.ttf", font_config);
	NUKLEUS_ASSERT(monospace != nullptr);
	// const auto default_font = atlas.get_default_font();
	// NUKLEUS_ASSERT(default_font != nullptr);

	nk::vec2<int> dimentions{};
	const void* image = atlas.bake_rgba32(dimentions);
	nk_sdl_device_upload_atlas(font_tex, image, dimentions.x, dimentions.y);
	tex_null = atlas.end(nk_handle_id(static_cast<int>(font_tex)));
}

}
