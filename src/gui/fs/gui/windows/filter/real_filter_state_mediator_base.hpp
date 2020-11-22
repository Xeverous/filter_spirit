#pragma once

#include <fs/gui/windows/filter/filter_state_mediator_base.hpp>
#include <fs/parser/parser.hpp>

namespace fs::gui {

class real_filter_state_mediator_base : public virtual filter_state_mediator_base
{
public:
	virtual void on_parsed_real_filter_change(
		const std::optional<parser::parsed_real_filter>& parsed_real_filter) = 0;

	virtual const std::optional<parser::parsed_real_filter>& parsed_real_filter() const = 0;
};

}
