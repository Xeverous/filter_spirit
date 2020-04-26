#include "platform/modal_dialogs.hpp"

#include "platform/windows/com_utils.hpp"
#include "platform/windows/string_utils.hpp"
#include "platform/windows/generic_utils.hpp"

#include <Shobjidl.h>

#include <string>
#include <string_view>
#include <exception>
#include <stdexcept>
#include <system_error>

namespace {

co_task_ptr<WCHAR> get_item_path(IShellItem& item)
{
	// https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitem-getdisplayname
	LPWSTR path;
	HRESULT hr = item.GetDisplayName(SIGDN_FILESYSPATH, &path);

	if (!SUCCEEDED(hr))
		throw_windows_error(hr, "IShellItem::GetDisplayName");

	return co_task_ptr<WCHAR>(path);
}

com_interface_ptr<IShellItem> shell_item_from_path(std::wstring const& path)
{
	void* item;
	HRESULT hr = SHCreateItemFromParsingName(path.c_str(), nullptr, IID_IShellItem, &item);

	if (!SUCCEEDED(hr))
		throw_windows_error(hr, "SHCreateItemFromParsingName");

	return com_interface_ptr<IShellItem>(static_cast<IShellItem*>(item));
}

com_interface_ptr<IShellItem> get_item_at(IShellItemArray& array, DWORD index)
{
	IShellItem* item;
	HRESULT hr = array.GetItemAt(index, &item);

	if (!SUCCEEDED(hr))
		throw_windows_error(hr, "IShellItemArray::GetItemAt");

	return com_interface_ptr<IShellItem>(item);
}

com_interface_ptr<IShellItem> get_dialog_result(IFileDialog& dialog)
{
	IShellItem* shell_item;
	HRESULT hr = dialog.GetResult(&shell_item);

	if (!SUCCEEDED(hr))
		throw_windows_error(hr, "IFileDialog::GetResult");

	return com_interface_ptr<IShellItem>(shell_item);
}

void apply_dialog_options(
	IFileDialog& dialog,
	filesystem_modal_settings const& fs_modal_st,
	bool confirm_overwrite,
	bool multiple_selection,
	bool follow_symlinks,
	bool path_must_exist,
	bool pick_folders)
{
	// Before setting, always get the options first in order not to override default options.
	FILEOPENDIALOGOPTIONS options;
	HRESULT hr = dialog.GetOptions(&options);
	if (!SUCCEEDED(hr))
		throw_windows_error(hr, "IFileDialog::GetOptions");

	if (fs_modal_st.allow_external_filesystem)
		options &= ~FOS_FORCEFILESYSTEM;
	else
		options |= FOS_FORCEFILESYSTEM;

	if (fs_modal_st.file_preview)
		options |= FOS_FORCEPREVIEWPANEON;
	else
		options &= ~FOS_FORCEPREVIEWPANEON;

	if (fs_modal_st.show_hidden_files)
		options |= FOS_FORCESHOWHIDDEN;
	else
		options &= ~FOS_FORCESHOWHIDDEN;

	if (confirm_overwrite)
		options |= FOS_OVERWRITEPROMPT;
	else
		options &= ~FOS_OVERWRITEPROMPT;

	if (multiple_selection)
		options |= FOS_ALLOWMULTISELECT;
	else
		options &= ~FOS_ALLOWMULTISELECT;

	if (follow_symlinks)
		options &= ~FOS_NODEREFERENCELINKS;
	else
		options |= FOS_NODEREFERENCELINKS;

	if (path_must_exist)
		options |= FOS_FILEMUSTEXIST;
	else
		options &= ~FOS_FILEMUSTEXIST;

	if (pick_folders)
		options |= FOS_PICKFOLDERS;
	else
		options &= ~FOS_PICKFOLDERS;

	hr = dialog.SetOptions(options);
	if (!SUCCEEDED(hr))
		throw_windows_error(hr, "IFileDialog::SetOptions");

	if (!fs_modal_st.window_title.empty())
	{
		hr = dialog.SetTitle(utf8_to_utf16(fs_modal_st.window_title).c_str());
		if (!SUCCEEDED(hr))
			throw_windows_error(hr, "IFileDialog::SetTitle");
	}

	if (!fs_modal_st.initial_directory.empty())
	{
		auto item = shell_item_from_path(utf8_to_utf16(fs_modal_st.initial_directory));
		hr = dialog.SetFolder(item.get());
		if (!SUCCEEDED(hr))
			throw_windows_error(hr, "IFileDialog::SetFolder");
	}

	for (auto const& path : fs_modal_st.additional_places)
	{
		auto item = shell_item_from_path(utf8_to_utf16(path));
		hr = dialog.AddPlace(item.get(), FDAP_BOTTOM);
		if (!SUCCEEDED(hr))
			throw_windows_error(hr, "IFileDialog::AddPlace");
	}
}

} // namespace

/*
 * Modal dialog code based on:
 * https://github.com/microsoft/Windows-classic-samples/tree/master/Samples/Win7Samples/winui/shell/appplatform/commonfiledialog
 * We dropped the CDialogEventHandler code because for simplicity elements
 * does not support reacting to the events while the modal is open.
 */
std::string
modal_dialog_save_file(
	native_window_handle window,
	const filesystem_modal_settings& fs_modal_st,
	const save_file_modal_settings& file_modal_st)
{
	// CoCreate the File Save Dialog object.
	auto dialog = make_instance<IFileSaveDialog>(CLSID_FileSaveDialog);
	apply_dialog_options(*dialog, fs_modal_st, file_modal_st.confirm_overwrite, false, true, true, false);

	if (!file_modal_st.initial_filename.empty())
	{
		HRESULT hr = dialog->SetFileName(utf8_to_utf16(file_modal_st.initial_filename).c_str());
		if (!SUCCEEDED(hr))
			throw_windows_error(hr, "IFileSaveDialog::SetFileName");
	}

	// Show the dialog
	// if the argument to show is a null pointer, the dialog is modeless
	HRESULT hr = dialog->Show(window);
	if (!SUCCEEDED(hr))
	{
		// user cancelled the operation or something else aborted the dialog
		return {};
	}

	return utf16_to_utf8(get_item_path(*get_dialog_result(*dialog)).get());
}

std::vector<std::string>
modal_dialog_open_file(
	native_window_handle window,
	const filesystem_modal_settings& fs_modal_st,
	const open_file_modal_settings& file_modal_st)
{
	// CoCreate the File Open Dialog object.
	auto dialog = make_instance<IFileOpenDialog>(CLSID_FileOpenDialog);
	apply_dialog_options(*dialog, fs_modal_st, false, file_modal_st.multiple_selection,
		file_modal_st.follow_symlinks, file_modal_st.path_must_exist, false);

	// Show the dialog
	// if the argument to show is a null pointer, the dialog is modeless
	HRESULT hr = dialog->Show(window);
	if (!SUCCEEDED(hr))
	{
		// user cancelled the operation or something else aborted the dialog
		return {};
	}

	// Obtain the result, once the user clicks the button.
	// The result is an IShellItemArray object.
	// https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifileopendialog-getresults
	auto items = [&]()
	{
		IShellItemArray* shell_items;
		hr = dialog->GetResults(&shell_items);

		if (!SUCCEEDED(hr))
			throw_windows_error(hr, "IFileDialog::GetResults");

		return com_interface_ptr<IShellItemArray>(shell_items);
	}();

	DWORD num_items;
	hr = items->GetCount(&num_items);
	if (!SUCCEEDED(hr))
		throw_windows_error(hr, "IShellItemArray::GetCount");

	std::vector<std::string> results;
	for (DWORD i = 0; i < num_items; ++i)
	{
		auto item = get_item_at(*items, i);
		auto path = get_item_path(*item);
		results.push_back(utf16_to_utf8(path.get()));
	}

	return results;
}

std::string
modal_dialog_open_directory(
	native_window_handle window,
	const filesystem_modal_settings& fs_modal_st)
{
	auto dialog = make_instance<IFileOpenDialog>(CLSID_FileOpenDialog);
	apply_dialog_options(*dialog, fs_modal_st, false, false, true, true, true);

	// Show the dialog
	// if the argument to show is a null pointer, the dialog is modeless
	HRESULT hr = dialog->Show(window);
	if (!SUCCEEDED(hr))
	{
		// user cancelled the operation or something else aborted the dialog
		return {};
	}

	return utf16_to_utf8(get_item_path(*get_dialog_result(*dialog)).get());
}
