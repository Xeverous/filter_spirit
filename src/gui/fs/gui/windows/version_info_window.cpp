#include <fs/gui/windows/version_info_window.hpp>
#include <fs/version.hpp>

#include <Magnum/GL/Context.h>
#include <Magnum/Version.h>
#include <Corrade/Version.h>

#include <imgui.h>

#include <boost/version.hpp>
#include <boost/config.hpp>

#include <initializer_list>

namespace fs::gui {

void version_info_window::draw_contents()
{
	constexpr auto line_width = 300;
	static const std::string fs_version = to_string(fs::version::current());
	static const std::string opengl_version = Magnum::GL::Context::current().versionString();
	static const std::string boost_version = std::to_string(BOOST_VERSION / 100000) + "." + std::to_string(BOOST_VERSION / 100 % 1000);
	static const std::string cpp_version = std::to_string(__cplusplus / 100);
	// __DATE__ has ugly "Mmm dd yyyy" format for legacy reasons, reorder it to ISO 8601
	const auto build_date = {
		__DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10], ' ',
		__DATE__[0], __DATE__[1], __DATE__[2], ' ',
		__DATE__[4], __DATE__[5], '\0'
	};

	ImGui::Text("Version information of FS and its dependencies:");

	ImGui::Text("Filter Spirit version:");
	ImGui::SameLine(line_width);
	ImGui::Text(fs_version.c_str());

	ImGui::Text("Boost version:");
	ImGui::SameLine(line_width);
	ImGui::Text(boost_version.c_str());

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
	ImGui::Text(opengl_version.c_str());

	ImGui::Text("\n\n\n"); // TODO: find a better way for vertical space

	ImGui::Text("build information:");

	ImGui::Text("build date:");
	ImGui::SameLine(line_width);
	ImGui::Text(build_date.begin());

	ImGui::Text("C++ standard:");
	ImGui::SameLine(line_width);
	ImGui::Text(cpp_version.c_str());

	ImGui::Text("C++ compiler:");
	ImGui::SameLine(line_width);
	ImGui::Text(BOOST_COMPILER);
}

}
