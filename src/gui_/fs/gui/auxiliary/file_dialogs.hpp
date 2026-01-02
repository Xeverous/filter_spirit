#pragma once

#include <string>
#include <optional>
#include <utility>

namespace fs::gui::aux {

struct file_dialog_result
{
	std::optional<std::string> file_path;
	std::optional<std::string> file_name;
	std::optional<std::string> file_content;
};

class open_file_dialog
{
public:
	void open_dialog(
		const char* title = nullptr,
		const char* patterns_desc = nullptr,
		const char* const* patterns_first = nullptr,
		const char* const* patterns_last = nullptr);

	bool is_dialog_opened() const
	{
		return _dialog_opened;
	}

	bool has_result() const
	{
		return _result.has_value();
	}

	std::optional<file_dialog_result> take_result()
	{
		return std::exchange(_result, std::nullopt);
	}

private:
	bool _dialog_opened = false;
	std::optional<file_dialog_result> _result;
};

}
