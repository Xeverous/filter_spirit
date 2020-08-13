#pragma once

#include <fs/log/monitor.hpp>
#include <fs/log/buffer_logger.hpp>

#include <elements/view.hpp>
#include <elements/element/tile.hpp>

#include <memory>
#include <utility>
#include <functional>

class gui_logger
{
public:
	gui_logger()
	: _monitor(_)
	{
	}

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

	      fs::log::monitor& monitor()       { return _monitor; }
	const fs::log::monitor& monitor() const { return _monitor; }

private:
	void update_ui();

	template <typename F>
	auto _logger(F f)
	{
		return _monitor([f](fs::log::logger& logger) {
			// this downcast is safe because the class knows
			// what logger type the monitor has been initialized with
			return f(static_cast<fs::log::buffer_logger&>(logger));
		});
	}

	fs::log::buffer_logger _; // should never be used directly
	fs::log::monitor _monitor;
	std::shared_ptr<cycfi::elements::vtile_composite> _ui_element;
	std::function<void()> _scroll_to_bottom;
};
