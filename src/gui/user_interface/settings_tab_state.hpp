#pragma once

#include <fs/network/download.hpp>

#include <elements/element/element.hpp>

#include <memory>

class settings_tab_state
{
public:
	std::shared_ptr<cycfi::elements::element> make_ui();

	const fs::network::download_settings& download_settings() const
	{
		return _download_settings;
	}

private:
	fs::network::download_settings _download_settings;
};
