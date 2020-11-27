#include <fs/gui/windows/application_log_window.hpp>

#include <imgui.h>

namespace fs::gui {

void application_log_window::draw_contents()
{
	ImGui::TextUnformatted(
		"Here you can find logs for the entire application.\n"
		"If some errors appear here, something must went horribly wrong.");
	_gui_logger.draw(_fonting.get(), _buffer_logger);
}

}
