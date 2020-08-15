#pragma once

#include <fs/network/download.hpp>

#include <elements/element/element.hpp>

#include <memory>

class settings_tab_state
{
public:
	std::shared_ptr<cycfi::elements::element> make_ui();

	const fs::network::network_settings& settings() const
	{
		return _settings;
	}

private:
	fs::network::network_settings _settings;
};
