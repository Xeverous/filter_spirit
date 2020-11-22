#pragma once

#include <fs/gui/windows/filter/real_filter_state_mediator_base.hpp>
#include <fs/gui/windows/filter/filter_state_mediator.hpp>

namespace fs::gui {

class real_filter_state_mediator
	: public virtual real_filter_state_mediator_base
	, public filter_state_mediator
{
public:
	real_filter_state_mediator(application& app)
	: filter_state_mediator(app)
	{
	}

	void on_source_change(const std::optional<std::string>& source) override;
	void on_parsed_real_filter_change(
		const std::optional<parser::parsed_real_filter>& parsed_real_filter) override;

	const std::optional<parser::parsed_real_filter>& parsed_real_filter() const override
	{
		return _parsed_real_filter;
	}

private:
	void new_parsed_real_filter(std::optional<parser::parsed_real_filter> parsed_real_filter);

	void draw_interface_derived() override;

	std::optional<parser::parsed_real_filter> _parsed_real_filter;
};

}
