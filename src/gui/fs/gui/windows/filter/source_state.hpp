#pragma once

#include <fs/gui/auxiliary/widgets.hpp>

#include <string>
#include <optional>

namespace fs::gui {

class filter_state_mediator_base;
class fonting;

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

	void open_text_input();

	void draw_interface(const fonting& f, filter_state_mediator_base& mediator);

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
	void on_text_input_done(filter_state_mediator_base& mediator);
	void on_text_input_cancel();

	std::optional<std::string> _source;
	std::optional<std::string> _file_path;
	aux::multiline_text_input _text_input;
	bool _popup_pending = false;
};

}
