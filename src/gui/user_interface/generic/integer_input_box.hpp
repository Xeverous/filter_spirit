#pragma once

#include <elements/element/element.hpp>

#include <memory>
#include <optional>

class integer_input_box
{
public:
	integer_input_box(int min_allowed, int max_allowed)
	: _min_allowed(min_allowed)
	, _max_allowed(max_allowed)
	, _root_element(make_ui())
	{
	}

	std::shared_ptr<cycfi::elements::element> ui()
	{
		return _root_element;
	}

	std::optional<int> value() const;

private:
	std::shared_ptr<cycfi::elements::element> make_ui();

	int _min_allowed;
	int _max_allowed;

	std::shared_ptr<cycfi::elements::element> _root_element;
};
