#include <fs/gui/windows/application_log_window.hpp>

#include <imgui.h>

namespace fs::gui {

void application_log_window::draw_contents()
{
	ImGui::TextUnformatted(
		"Here you can find logs for the entire application.\n"
		"If some errors appear here, something must went horribly wrong.");

	_logger.draw(-1.5f); // take as much space as possible but leave 1.5 lines of space for buttons
}

}
