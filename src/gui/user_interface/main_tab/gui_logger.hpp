#pragma once

#include <fs/log/thread_safe_logger.hpp>
#include <fs/log/buffer_logger.hpp>

#include <elements/element.hpp>

#include <memory>
#include <utility>
#include <functional>

class gui_logger
{
public:
	std::shared_ptr<cycfi::elements::element> make_ui();

	void update();

	void clear_logs();

	      auto& logger()       { return _logger; }
	const auto& logger() const { return _logger; }

private:
	fs::log::thread_safe_logger<fs::log::buffer_logger> _logger;
	std::shared_ptr<cycfi::elements::vtile_composite> _composite;
	std::function<void()> _scroll_to_bottom;
};
