#pragma once

#ifdef FILTER_SPIRIT_PLATFORM_WINDOWS
#include <Windows.h>
using native_window_handle = HWND;
#elif FILTER_SPIRIT_PLATFORM_GTK
	#error not implemented
#elif
	#error no FILTER_SPIRIT_PLATFORM_* set
#endif

#include <string>
#include <vector>

/*
 * Taken from https://github.com/cycfi/elements/issues/44 which has a WIP implementation:
 * https://github.com/Xeverous/elements/commit/5e6e6d74a130ca705492c0bd3c1f7157645939f4
 * The patch will take some time before being merged into elements library.
 */
struct filesystem_modal_settings
{
	/*
	 * Title of the modal window. If empty, platform default is used.
	 *
	 * Windows: uses IFileDialog::SetTitle
	 * macOS: uses NSSavePanel::title and NSOpenPanel::title
	 * GTK: uses gtk_file_chooser_dialog_new
	 *
	 * Defaults:
	 * GTK: program name
	 * Windows: "Save As" / "Open" / "Select Folder" strings (localized by user settings)
	 * macOS: TODO
	 */
	std::string window_title = {};
	/*
	 * Initial directory from which the browsing starts. If empty,
	 * platform default is used, which often is the place used previously.
	 *
	 * Windows: uses IFileDialog::SetFolder
	 * macOS: uses NSSavePanel::directoryURL or NSOpenPanel::directoryURL
	 * GTK: uses gtk_file_chooser_set_current_folder
	 *
	 * implementation notes:
	 * - both GTK and Windows require absolute paths
	 * - both GTK and Windows discourage use of this option, they recommend
	 *	to rely on implicit defaults in the underlying implementation
	 */
	std::string initial_directory = {}; // TODO fs::path?
	/*
	 * Additional places proposed in the modal window.
	 *
	 * Windows: uses IFileDialog::AddPlace
	 * macOS: ignored
	 * GTK: uses gtk_file_chooser_add_shortcut_folder
	 *
	 * implementation notes:
	 * - both GTK and Windows require absolute paths
	 */
	std::vector<std::string> additional_places; // TODO fs::path?
	/*
	 * If true, the modal will allow to select files outside local
	 * filesystem (eg OS-integrated cloud storage).
	 *
	 * Windows: uses FOS_FORCEFILESYSTEM
	 * macOS: TODO
	 * GTK: uses gtk_file_chooser_set_local_only
	 *
	 * TODO this option is very likely to cause problems when
	 * selected items do not have standard paths. GTK and Windows have the
	 * concept of file URI but I have no idea whether they work the same.
	 */
	bool allow_external_filesystem = false;
	/*
	 * Show a file preview window in the modal. Ignored on platforms
	 * where there is no control of it.
	 *
	 * Windows: uses FOS_FORCEPREVIEWPANEON (the user can still click to hide it)
	 * macOS: ignored
	 * GTK: ignored, not implemented
	 *
	 * TODO should this option exist at all?
	 */
	bool file_preview = true;
	/*
	 * Show hidden files.
	 *
	 * Windows: uses FOS_FORCESHOWHIDDEN
	 * macOS: uses showsHiddenFiles in TODO panel
	 * GTK: uses gtk_file_chooser_set_show_hidden
	 */
	bool show_hidden_files = false;
	/*
	 * Show an expanded version of the modal.
	 *
	 * Windows: ignored
	 * macOS: expanded propety in NSSavePanel and NSOpenPanel
	 * GTK: ignored
	 */
	bool expanded = true;
};

struct save_file_modal_settings
{
	/*
	 * Prompt the user for confirmation when an existing
	 * file path is selected.
	 *
	 * Windows: uses FOS_OVERWRITEPROMPT
	 * macOS: ignored
	 * GTK: uses gtk_file_chooser_set_do_overwrite_confirmation
	 *
	 * Implementation notes:
	 * - Windows default is to prompt for confirmation, GTK is to not
	 */
	bool confirm_overwrite = true;
	/*
	 * Initial filename that is entered in the modal. Intended
	 * for application-specific proposals like "Untitled Document".
	 *
	 * The name should not contain any extension.
	 *
	 * Implementation notes:
	 * - Windows expects no extension (appends default filter automatically)
	 * - GTK expects full filename (name + extension) - perhaps append the default filter?
	 */
	std::string initial_filename = {};
	/*
	 * TODO allowed extensions
	 */
	//void x;
	/*
	 * TODO
	 */
	//std::string default_extension;
	/*
	 * Allow the user to use a different extension than one of the proposed.
	 *
	 * TODO
	 */
	//bool allow_other_extension = true;
};

struct open_file_modal_settings
{
	/*
	 * Allow the user to select multiple files
	 *
	 * Windows: uses FOS_ALLOWMULTISELECT
	 * macOS: uses allowsMultipleSelection
	 * GTK: uses gtk_file_chooser_set_select_multiple
	 */
	bool multiple_selection = false;
	/*
	 * TODO filterting
	 */
	//void x;
	/*
	 * TODO default filter
	 */
	//void y;
	/*
	 * When set to false, symlinks will be opened as files
	 * instead of following them.
	 *
	 * Windows: uses FOS_NODEREFERENCELINKS
	 * macOS: uses resolvesAliases
	 * GTK: ignored (no support)
	 */
	bool follow_symlinks = true;
	/*
	 * Require the user to select a path that exists.
	 *
	 * Windows: uses FOS_FILEMUSTEXIST
	 * macOS: ignored, must always exist
	 * GTK: ignored, must always exist
	 *
	 * TODO is this option useful at all?
	 */
	bool path_must_exist = true;
};

[[nodiscard]] std::string
modal_dialog_save_file(
	native_window_handle window,
	const filesystem_modal_settings&,
	const save_file_modal_settings&);

[[nodiscard]] std::vector<std::string>
modal_dialog_open_file(
	native_window_handle window,
	const filesystem_modal_settings&,
	const open_file_modal_settings&);

[[nodiscard]] std::string
modal_dialog_open_directory(
	native_window_handle window,
	const filesystem_modal_settings&);

