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
	{
	}

	std::shared_ptr<cycfi::elements::element> make_ui();

	std::optional<int> value() const;

private:
	int _min_allowed;
	int _max_allowed;
};
