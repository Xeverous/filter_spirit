#pragma once

#include <fs/log/thread_safe_logger.hpp>
#include <fs/log/buffer_logger.hpp>

#include <elements/view.hpp>
#include <elements/element/tile.hpp>

#include <memory>
#include <utility>
#include <functional>

class gui_logger
{
public:
	// This is a bit ugly (should be done in the constructor) but needs to be done
	// later to avoid cyclic dependency between state and UI code.
	void set_ui_elements(std::shared_ptr<cycfi::elements::vtile_composite> ui_element, std::function<void()> scroll_to_bottom)
	{
		_ui_element = std::move(ui_element);
		_scroll_to_bottom = std::move(scroll_to_bottom);
	}

	void update()
	{
		update_ui();
	}

	void clear_logs();

	      auto& logger()       { return _logger; }
	const auto& logger() const { return _logger; }

private:
	void update_ui();

	fs::log::thread_safe_logger<fs::log::buffer_logger> _logger;
	std::shared_ptr<cycfi::elements::vtile_composite> _ui_element;
	std::function<void()> _scroll_to_bottom;
};
