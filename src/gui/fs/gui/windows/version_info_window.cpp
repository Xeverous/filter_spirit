#include <fs/gui/windows/version_info_window.hpp>
#include <fs/version.hpp>

#include <Magnum/GL/Context.h>
#include <Magnum/Version.h>
#include <Corrade/Version.h>

#include <imgui.h>

#include <SDL_version.h>

#include <curl/curl.h>

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
	const auto build_date = {
		__DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10], ' ',
		__DATE__[0], __DATE__[1], __DATE__[2], ' ',
		__DATE__[4], __DATE__[5], '\0'
	};

	ImGui::Text("Version information of FS and its dependencies:");

	ImGui::Text("Filter Spirit version:");
	ImGui::SameLine(line_width);
	ImGui::Text(_fs_version.c_str());

	ImGui::Text("Boost version:");
	ImGui::SameLine(line_width);
	ImGui::Text(_boost_version.c_str());

	ImGui::Text("Magnum version:");
	ImGui::SameLine(line_width);
	ImGui::Text(MAGNUM_VERSION_STRING);

	ImGui::Text("Corrade version:");
	ImGui::SameLine(line_width);
	ImGui::Text(CORRADE_VERSION_STRING);

	ImGui::Text("Dear ImGui version:");
	ImGui::SameLine(line_width);
	ImGui::Text("%s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);

	ImGui::Text("OpenGL version:");
	ImGui::SameLine(line_width);
	ImGui::Text(_opengl_version.c_str());

	ImGui::Text("SDL version:");
	ImGui::SameLine(line_width);
	ImGui::Text(_sdl_version.c_str());

	ImGui::Text("libcurl version:");
	ImGui::SameLine(line_width);
	ImGui::Text(curl_version());

	ImGui::Dummy({0.0f, ImGui::GetFontSize() * 2.0f});

	ImGui::Text("Build information:");

	ImGui::Text("build date:");
	ImGui::SameLine(line_width);
	ImGui::Text(build_date.begin());

	ImGui::Text("C++ standard:");
	ImGui::SameLine(line_width);
	ImGui::Text(_cpp_version.c_str());

	ImGui::Text("C++ compiler:");
	ImGui::SameLine(line_width);
	ImGui::Text(BOOST_COMPILER);
}

}
