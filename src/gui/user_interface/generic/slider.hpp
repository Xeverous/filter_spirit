#pragma once

#include <elements/element.hpp>

#include <functional>
#include <memory>

class slider
{
public:
	slider(double init_value, std::function<void(double)> on_change);

	std::shared_ptr<cycfi::elements::element> ui()
	{
		return _root_element;
	}

private:
	void make_ui(double init_value, std::function<void(double)> on_change);

	std::shared_ptr<cycfi::elements::element> _root_element;
};
