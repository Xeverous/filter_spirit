// has to be first in order to pull implementation before header guards from other indirect includes
#define NK_IMPLEMENTATION
#include <nukleus.hpp>

#include "application.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <memory>
#include <chrono>
#include <thread>

namespace {

int nk_sdl_handle_event(nk::event_input& input, SDL_Event& evt)
{
	switch (evt.type)
	{
		case SDL_KEYUP: /* KEYUP & KEYDOWN share same routine */
		case SDL_KEYDOWN:
		{
			const int down = evt.type == SDL_KEYDOWN;
			const int ctrl_down = SDL_GetModState() & KMOD_CTRL;
			static int insert_toggle = 0;

			switch (evt.key.keysym.sym)
			{
				case SDLK_RSHIFT: /* RSHIFT & LSHIFT share same routine */
				case SDLK_LSHIFT:    input.key(NK_KEY_SHIFT, down); break;
				case SDLK_DELETE:    input.key(NK_KEY_DEL, down); break;

				case SDLK_KP_ENTER:
				case SDLK_RETURN:    input.key(NK_KEY_ENTER, down); break;

				case SDLK_TAB:       input.key(NK_KEY_TAB, down); break;
				case SDLK_BACKSPACE: input.key(NK_KEY_BACKSPACE, down); break;
				case SDLK_HOME:      input.key(NK_KEY_TEXT_START, down);
				                     input.key(NK_KEY_SCROLL_START, down); break;
				case SDLK_END:       input.key(NK_KEY_TEXT_END, down);
				                     input.key(NK_KEY_SCROLL_END, down); break;
				case SDLK_PAGEDOWN:  input.key(NK_KEY_SCROLL_DOWN, down); break;
				case SDLK_PAGEUP:    input.key(NK_KEY_SCROLL_UP, down); break;
				case SDLK_z:         input.key(NK_KEY_TEXT_UNDO, down && ctrl_down); break;
				case SDLK_r:         input.key(NK_KEY_TEXT_REDO, down && ctrl_down); break;
				case SDLK_c:         input.key(NK_KEY_COPY, down && ctrl_down); break;
				case SDLK_v:         input.key(NK_KEY_PASTE, down && ctrl_down); break;
				case SDLK_x:         input.key(NK_KEY_CUT, down && ctrl_down); break;
				case SDLK_b:         input.key(NK_KEY_TEXT_LINE_START, down && ctrl_down); break;
				case SDLK_e:         input.key(NK_KEY_TEXT_LINE_END, down && ctrl_down); break;
				case SDLK_UP:        input.key(NK_KEY_UP, down); break;
				case SDLK_DOWN:      input.key(NK_KEY_DOWN, down); break;
				case SDLK_ESCAPE:    input.key(NK_KEY_TEXT_RESET_MODE, down); break;
				case SDLK_INSERT:
					if (down)
						insert_toggle = !insert_toggle;
					input.key(insert_toggle ? NK_KEY_TEXT_INSERT_MODE : NK_KEY_TEXT_REPLACE_MODE, down);
					break;
				case SDLK_a:
					if (ctrl_down)
						input.key(NK_KEY_TEXT_SELECT_ALL, down);
					break;
				case SDLK_LEFT:
					input.key(ctrl_down ? NK_KEY_TEXT_WORD_LEFT : NK_KEY_LEFT, down);
					break;
				case SDLK_RIGHT:
					input.key(ctrl_down ? NK_KEY_TEXT_WORD_RIGHT : NK_KEY_RIGHT, down);
					break;
			}
		}
		return 1;

		/* MOUSEBUTTONUP & MOUSEBUTTONDOWN share same routine */
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
		{
			const int down = evt.type == SDL_MOUSEBUTTONDOWN;
			const int x = evt.button.x, y = evt.button.y;
			switch (evt.button.button)
			{
				case SDL_BUTTON_LEFT:
					if (evt.button.clicks > 1)
						input.button(NK_BUTTON_DOUBLE, x, y, down);
					input.button(NK_BUTTON_LEFT, x, y, down); break;
				case SDL_BUTTON_MIDDLE: input.button(NK_BUTTON_MIDDLE, x, y, down); break;
				case SDL_BUTTON_RIGHT:  input.button(NK_BUTTON_RIGHT, x, y, down); break;
			}
		}
		return 1;

		case SDL_MOUSEMOTION:
			if (input.get().mouse.grabbed) {
				const auto x = static_cast<int>(input.get().mouse.prev.x);
				const auto y = static_cast<int>(input.get().mouse.prev.y);
				input.motion(x + evt.motion.xrel, y + evt.motion.yrel);
			}
			else {
				input.motion(evt.motion.x, evt.motion.y);
			}
			return 1;

		case SDL_TEXTINPUT:
		{
			nk_glyph glyph;
			memcpy(glyph, evt.text.text, NK_UTF_SIZE);
			input.glyph(glyph);
			return 1;
		}

		case SDL_MOUSEWHEEL:
			input.scroll({evt.wheel.preciseX, evt.wheel.preciseY});
			return 1;
	}
	return 0;
}

void nk_sdl_handle_grab(nk_input& input, SDL_Window& win)
{
	if (input.mouse.grab) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	else if (input.mouse.ungrab) {
		/* better support for older SDL by setting mode first; causes an extra mouse motion event */
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_WarpMouseInWindow(&win, static_cast<int>(input.mouse.prev.x), static_cast<int>(input.mouse.prev.y));
	}
	else if (input.mouse.grabbed) {
		input.mouse.pos.x = input.mouse.prev.x;
		input.mouse.pos.y = input.mouse.prev.y;
	}
}

struct nk_sdl_vertex
{
	float position[2];
	float uv[2];
	nk_byte col[4];
};

struct buffers
{
	nk::buffer cmds = nk::buffer::init_default();
	nk::buffer vbuf = nk::buffer::init_default();
	nk::buffer ebuf = nk::buffer::init_default();
};

void nk_sdl_render(nk::context& ctx, buffers& buffs, nk_draw_null_texture tex_null, SDL_Window& win, nk_anti_aliasing aa, Uint64& time_of_last_frame)
{
	/* setup global state */
	const Uint64 now = SDL_GetTicks64();
	ctx.get_delta_time_seconds() = static_cast<float>(now - time_of_last_frame) / 1000;
	time_of_last_frame = now;

	int width{}, height{};
	SDL_GetWindowSize(&win, &width, &height);
	int display_width{}, display_height{};
	SDL_GL_GetDrawableSize(&win, &display_width, &display_height);

	struct nk_vec2 scale;
	scale.x = static_cast<float>(display_width) / static_cast<float>(width);
	scale.y = static_cast<float>(display_height) / static_cast<float>(height);

	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* setup viewport/project */
	glViewport(0, 0, static_cast<GLsizei>(display_width), static_cast<GLsizei>(display_height));
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	{
		/* convert from command queue into draw list and draw to screen */

		/* fill converting configuration */
		static const nk_draw_vertex_layout_element vertex_layout[] = {
			{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(nk_sdl_vertex, position)},
			{NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(nk_sdl_vertex, uv)},
			{NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(nk_sdl_vertex, col)},
			{NK_VERTEX_LAYOUT_END}
		};

		/* convert shapes into vertexes */
		nk_convert_config config{};
		config.vertex_layout = vertex_layout;
		config.vertex_size = sizeof(nk_sdl_vertex);
		config.vertex_alignment = alignof(nk_sdl_vertex);
		config.tex_null = tex_null;
		config.circle_segment_count = 22;
		config.curve_segment_count = 22;
		config.arc_segment_count = 22;
		config.global_alpha = 1.0f;
		config.shape_AA = aa;
		config.line_AA = aa;
		const nk::convert_result_flags result = ctx.convert(buffs.cmds, buffs.vbuf, buffs.ebuf, config);
		if (result != nk::convert_result_flags::success)
			SDL_Log("Error when converting: %d\n", static_cast<int>(result));

		/* setup vertex buffer pointer */
		{
			constexpr GLsizei vs = sizeof(nk_sdl_vertex);
			const size_t vp = offsetof(nk_sdl_vertex, position);
			const size_t vt = offsetof(nk_sdl_vertex, uv);
			const size_t vc = offsetof(nk_sdl_vertex, col);
			const void* vertices = buffs.vbuf.memory();
			glVertexPointer  (2, GL_FLOAT,         vs, static_cast<const nk_byte*>(vertices) + vp);
			glTexCoordPointer(2, GL_FLOAT,         vs, static_cast<const nk_byte*>(vertices) + vt);
			glColorPointer   (4, GL_UNSIGNED_BYTE, vs, static_cast<const nk_byte*>(vertices) + vc);
		}

		/* iterate over and execute each draw command */
		auto offset = static_cast<const nk_draw_index*>(buffs.ebuf.memory());
		for (const nk_draw_command& cmd : ctx.draw_commands(buffs.cmds))
		{
			if (!cmd.elem_count)
				continue;

			glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(cmd.texture.id));
			glScissor(
				static_cast<GLint>(cmd.clip_rect.x * scale.x),
				static_cast<GLint>((height - static_cast<GLint>(cmd.clip_rect.y + cmd.clip_rect.h)) * scale.y),
				static_cast<GLint>(cmd.clip_rect.w * scale.x),
				static_cast<GLint>(cmd.clip_rect.h * scale.y));
			#ifdef NK_UINT_DRAW_INDEX
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cmd.elem_count), GL_UNSIGNED_INT, offset);
			#else
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cmd.elem_count), GL_UNSIGNED_SHORT, offset);
			#endif

			offset += cmd.elem_count;
		}

		ctx.clear();
		buffs.cmds.clear();
		buffs.vbuf.clear();
		buffs.ebuf.clear();
	}

	/* default OpenGL state */
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

void nk_sdl_clipboard_paste(nk_handle /*usr*/, nk_text_edit* edit)
{
	char* text = SDL_GetClipboardText();
	if (text)
		nk_textedit_paste(edit, text, nk_strlen(text));

	SDL_free(text); // NK does not always free it but SDL docs says it is always required
}

void nk_sdl_clipboard_copy(nk_handle /*usr*/, const char* text, int len)
{
	if (!len)
		return;

	auto str = static_cast<char*>(malloc(static_cast<size_t>(len + 1)));
	if (!str)
		return;

	memcpy(str, text, static_cast<size_t>(len));
	str[len] = '\0';

	SDL_SetClipboardText(str);
	free(str);
}

} // namespace

struct sdl_window_deleter
{
	void operator()(SDL_Window* win) const
	{
		SDL_DestroyWindow(win);
	}
};

struct sdl_context_deleter
{
	void operator()(SDL_GLContext ctx) const
	{
		SDL_GL_DeleteContext(ctx);
	}
};

int main()
{
	// SDL setup
	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
	if (const auto err = SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("SDL initialization failed: %s\n", SDL_GetError());
		return err;
	}
	auto _1 = nk::finally(&SDL_Quit);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	auto win = std::unique_ptr<SDL_Window, sdl_window_deleter>(SDL_CreateWindow(
		"Filter Spirit GUI TEST", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1200, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE));

	auto glContext = std::unique_ptr<void, sdl_context_deleter>(SDL_GL_CreateContext(win.get()));

	if (SDL_GL_SetSwapInterval(1) != 0)
		SDL_Log("VSync failed: %s\n", SDL_GetError());

	fs::gui::ui_state uis;
	SDL_GetWindowSize(win.get(), &uis.window_width, &uis.window_height);

	glGenTextures(1, &uis.font_tex);
	auto _2 = nk::finally([&uis](){ glDeleteTextures(1, &uis.font_tex); });

	uis.regenerate_font_atlas();

	auto ctx = nk::context::init_default(uis.fontin_regular->handle);
	NUKLEUS_ASSERT(ctx.is_valid());
	ctx.get_clipboard().copy = nk_sdl_clipboard_copy;
	ctx.get_clipboard().paste = nk_sdl_clipboard_paste;
	ctx.get_clipboard().userdata = nk_handle_ptr(nullptr);

	buffers buffs;
	Uint64 time_of_last_frame = SDL_GetTicks64();

	const nk::colorf bg(0.0f, 0.0f, 0.0f);
	// This is here because nk_default_color_style is only exposed under NK_IMPLEMENTATION.
	// Otherwise it would be moved to inside of style_configurator().
	//nk::color_table color_table(nk_default_color_style);
	//nk::color_table default_color_table(nk_default_color_style);

	fs::gui::application app;

	while (true)
	{
		/* Input */
		SDL_Event evt;
		{
			auto input = ctx.input_scoped();
			while (SDL_PollEvent(&evt)) {
				if (evt.type == SDL_QUIT)
					return 0;

				nk_sdl_handle_event(input, evt);
			}
			nk_sdl_handle_grab(ctx.get_input(), *win); /* optional grabbing behavior */
		}

		/* GUI */
		app.draw(ctx, uis);

		/* Draw */
		SDL_GetWindowSize(win.get(), &uis.window_width, &uis.window_height);
		glViewport(0, 0, uis.window_width, uis.window_height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(bg.r, bg.g, bg.b, bg.a);
		/* IMPORTANT: `nk_sdl_render` modifies some global OpenGL state
		 * with blending, scissor, face culling, depth test and viewport and
		 * defaults everything back into a default state.
		 * Make sure to either a.) save and restore or b.) reset your own state after rendering the UI. */
		nk_sdl_render(ctx, buffs, uis.tex_null, *win, NK_ANTI_ALIASING_ON, time_of_last_frame);
		SDL_GL_SwapWindow(win.get());
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	return 0;
}
