#pragma once

#include <fs/gui/auxiliary/widgets.hpp>

#include <string>
#include <optional>
#include <utility>

namespace fs::gui {

class filter_state_mediator;
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

	void open_source_edit();

	void draw_interface(const fonting& f, filter_state_mediator& mediator);

	const std::string* source() const
	{
		return _source.has_value() ? &*_source : nullptr;
	}

	const std::string* path() const
	{
		return _file_path.has_value() ? &*_file_path : nullptr;
	}

	const std::string* name() const
	{
		return _file_name.has_value() ? &*_file_name : nullptr;
	}

	void name(std::optional<std::string> nm)
	{
		_file_name = std::move(nm);
	}

	bool is_source_edited() const
	{
		return _is_source_edited;
	}

// These are not available in emscripten builds because there is no way to load user files there
// (JavaScript has no access to user's files)
#ifndef __EMSCRIPTEN__
	// use string instead of string_view because filesystem wants C-strings
	void load_source_file(std::string path, filter_state_mediator& mediator);
	void reload_source_file(filter_state_mediator& mediator);
#endif

	void new_source(std::optional<std::string> source, filter_state_mediator& mediator);

private:
	void on_text_input_done(filter_state_mediator& mediator);
	void on_text_input_cancel();

	std::optional<std::string> _source;
	std::optional<std::string> _file_path;
	std::optional<std::string> _file_name;
	aux::multiline_text_input _text_input;
	bool _popup_pending = false;
	bool _is_source_edited = false;
};

}
