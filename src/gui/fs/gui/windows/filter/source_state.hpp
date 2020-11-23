#pragma once

#include <string>
#include <optional>

namespace fs::gui {

class filter_state_mediator_base;

/**
 * @class represents text input to the program
 * which can either be from file or from memory
 * such as UI's multiline text edit
 */
class source_state
{
public:
	bool is_from_file() const
	{
		return _file_path.has_value();
	}

	void draw_interface(filter_state_mediator_base& mediator);

	const std::optional<std::string>& source() const
	{
		return _source;
	}

	const std::optional<std::string>& source_path() const
	{
		return _file_path;
	}

	// use string instead of string_view because filesystem wants C-strings
	void load_source_file(std::string path, filter_state_mediator_base& mediator);
	void reload_source_file(filter_state_mediator_base& mediator);

private:
	void new_source(std::optional<std::string> source, filter_state_mediator_base& mediator);

	std::optional<std::string> _source;
	std::optional<std::string> _file_path;
};

}
