#pragma once

#include <elements/element.hpp>

class loot_preview_state
{
public:
	std::shared_ptr<cycfi::elements::element> make_ui();

private:
	cycfi::elements::flow_composite _flow_composite;
};
