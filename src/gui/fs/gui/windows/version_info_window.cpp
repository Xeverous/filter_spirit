#include <fs/gui/windows/version_info_window.hpp>
#include <fs/version.hpp>

#include <Magnum/GL/Context.h>
#include <Magnum/version.h>
#include <Corrade/version.h>

#include <imgui.h>

#include <SDL_version.h>

#ifndef __EMSCRIPTEN__
#include <curl/curl.h>
#endif

#include <boost/version.hpp>
#include <boost/config.hpp>

#include <initializer_list>
#include <string>

namespace {

std::string sdl_version_string()
{
	/*
	 * Note: SDL has 2 ways of obtaining version information:
	 * - SDL_GetVersion - library call
	 * - SDL_VERSION - macro
	 *
	 * The first one returns version which the program is linked against,
	 * the second one returns version which the program was compiled with.
	 *
	 * FS does not come with SDL bundled with it (except on Windows for convenience and ABI) so it is
	 * better to query version information at runtime, because the library can be upgraded anytime.
	 */
	SDL_version ver;
	SDL_GetVersion(&ver);
	return std::to_string(ver.major) + "." + std::to_string(ver.minor) + "." + std::to_string(ver.patch);
}

}

namespace fs::gui {

version_info_window::version_info_window()
: imgui_window("Version information")
, _fs_version(to_string(fs::version::current()))
, _boost_version(std::to_string(BOOST_VERSION / 100000) + "." + std::to_string(BOOST_VERSION / 100 % 1000))
, _opengl_version(Magnum::GL::Context::current().versionString())
, _sdl_version(sdl_version_string())
, _cpp_version(std::to_string(__cplusplus / 100))
{
}

void version_info_window::draw_contents()
{
	const auto line_width = ImGui::GetFontSize() * 15.0f;
	// __DATE__ has ugly "Mmm dd yyyy" format for legacy reasons, reorder it to ISO 8601
	// also when dd < 10 change space to 0 to avoid ugly double space
	const auto build_date = {
		__DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10], ' ',
		__DATE__[0], __DATE__[1], __DATE__[2], ' ',
		__DATE__[4] == ' ' ? '0' : __DATE__[4], __DATE__[5], '\0'
	};

	ImGui::TextUnformatted("Version information of FS and its dependencies:");

	ImGui::TextUnformatted("Filter Spirit version:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(_fs_version.c_str());

	ImGui::TextUnformatted("Boost version:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(_boost_version.c_str());

	ImGui::TextUnformatted("Magnum version:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(MAGNUM_VERSION_STRING);

	ImGui::TextUnformatted("Corrade version:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(CORRADE_VERSION_STRING);

	ImGui::TextUnformatted("Dear ImGui version:");
	ImGui::SameLine(line_width);
	ImGui::Text("%s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);

	ImGui::TextUnformatted("OpenGL version:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(_opengl_version.c_str());

	ImGui::TextUnformatted("SDL version:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(_sdl_version.c_str());

#ifdef __EMSCRIPTEN__
	ImGui::TextUnformatted("Emscripten version:");
	ImGui::SameLine(line_width);
	ImGui::Text("%d.%d.%d", __EMSCRIPTEN_major__, __EMSCRIPTEN_minor__, __EMSCRIPTEN_tiny__);
#else
	ImGui::TextUnformatted("libraries for networking:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(curl_version());
#endif

	ImGui::Dummy({0.0f, ImGui::GetFontSize() * 2.0f});

	ImGui::TextUnformatted("Build information:");

	ImGui::TextUnformatted("build date:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(build_date.begin());

	ImGui::TextUnformatted("C++ standard:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(_cpp_version.c_str());

	ImGui::TextUnformatted("C++ compiler:");
	ImGui::SameLine(line_width);
	ImGui::TextUnformatted(BOOST_COMPILER);
}

}
