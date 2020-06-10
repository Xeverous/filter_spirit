#pragma once

#include <fs/log/logger.hpp>
#include <fs/log/string_logger.hpp>

#include <elements/view.hpp>
#include <elements/element/tile.hpp>

#include <memory>
#include <utility>

class gui_logger : public fs::log::logger
{
public:
	// This is a bit ugly (should be done in the constructor) but needs to be done
	// later to avoid cyclic dependency between state and UI code.
	void set_ui_elements(std::shared_ptr<cycfi::elements::vtile_composite> ui_element, cycfi::elements::view& view)
	{
		_ui_element = std::move(ui_element);
		_view = &view;
	}

	void begin_message(fs::log::severity s) override
	{
		_severity = s;
	}

	void end_message() override;

	void add(std::string_view text) override
	{
		_string_logger.add(text);
	}

	void add(char character) override
	{
		_string_logger.add(character);
	}

	void add(std::int64_t number) override
	{
		_string_logger.add(number);
	}

	void add(std::uint64_t number) override
	{
		_string_logger.add(number);
	}

private:
	fs::log::severity _severity = fs::log::severity::info;
	fs::log::string_logger _string_logger;
	std::shared_ptr<cycfi::elements::vtile_composite> _ui_element;
	cycfi::elements::view* _view = nullptr;
};
