#pragma once

#include <fs/network/download.hpp>

#include <elements/element/element.hpp>

#include <memory>

class settings_tab_state
{
public:
	settings_tab_state()
	: _root_element(make_ui())
	{
	}

	std::shared_ptr<cycfi::elements::element> ui()
	{
		return _root_element;
	}

	const fs::network::download_settings& download_settings() const
	{
		return _download_settings;
	}

private:
	std::shared_ptr<cycfi::elements::element> make_ui();

	fs::network::download_settings _download_settings;

	std::shared_ptr<cycfi::elements::element> _root_element;
};
