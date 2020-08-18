#include "user_interface/main_tab/builtin_options/api_selection_state.hpp"

#include <elements/element.hpp>

namespace el = cycfi::elements;

namespace {

constexpr auto poe_ninja = "poe.ninja";
constexpr auto poe_watch = "poe.watch";
constexpr auto none = "none";

}

std::shared_ptr<el::element> api_selection_state::make_ui()
{
	return el::share(el::selection_menu(
		[this](std::string_view selected) {
			if (selected == poe_ninja)
				_selected_api = fs::lang::data_source_type::poe_ninja;
			else if (selected == poe_watch)
				_selected_api = fs::lang::data_source_type::poe_watch;
			else
				_selected_api = fs::lang::data_source_type::none;

			_inserter.push_event(events::api_selection_changed{});
		},
		{ poe_ninja, poe_watch, none }
	).first);
}
