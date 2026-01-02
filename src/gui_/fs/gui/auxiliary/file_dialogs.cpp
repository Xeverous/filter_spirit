#include <fs/gui/auxiliary/file_dialogs.hpp>

#ifndef __EMSCRIPTEN__
#include <tinyfiledialogs.h>
#endif

namespace fs::gui::aux {

void open_file_dialog::open_dialog(
	const char* title,
	const char* patterns_desc,
	const char* const* patterns_first,
	const char* const* patterns_last)
{
#ifdef __EMSCRIPTEN__
#else
	const char* const selected_path = tinyfd_openFileDialog(
		/* title */ title,
		/* default path */ nullptr,
		/* number of filter patterns */ patterns_last - patterns_first,
		/* filter patterns */ patterns_first,
		/* filter pattern description */ patterns_desc,
		/* multiselect allowed? */ 0);

	file_dialog_result result;

	if (selected_path)
		result.file_path = selected_path;

	_result = std::move(result);
#endif
}

}
