#pragma once

#include <fs/lang/item_filter.hpp>
#include <fs/log/logger.hpp>

#include <string>
#include <optional>

namespace fs::gui {

class application;

class filter_state_mediator_base
{
public:
	virtual ~filter_state_mediator_base() = default;

	virtual void on_source_change(const std::optional<std::string>& source) = 0;
	virtual void on_filter_representation_change(const std::optional<lang::item_filter>& filter_representation) = 0;

	virtual const std::optional<std::string>& source() const = 0;
	virtual const std::optional<std::string>& source_path() const = 0;
	virtual const std::optional<lang::item_filter>& filter_representation() const = 0;

	virtual log::logger& logger() = 0;

	virtual void draw_interface(application& app) = 0;
};

}
