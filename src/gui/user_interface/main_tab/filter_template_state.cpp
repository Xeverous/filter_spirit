#include "user_interface/main_tab/filter_template_state.hpp"
#include "platform/modal_dialogs.hpp"
#include "ui_utils.hpp"

#include <fs/generator/generate_filter.hpp>
#include <fs/generator/make_filter.hpp>
#include <fs/utility/file.hpp>

namespace el = cycfi::elements;

std::shared_ptr<el::element> filter_template_state::make_ui(el::host_window_handle window)
{
	_label_filter_template_path = el::share(el::label("(no filter template selected)"));

	auto on_filter_template_browse = [window, this](bool) mutable {
		filesystem_modal_settings settings;
		open_file_modal_settings of_settings;
		std::vector<std::string> paths = modal_dialog_open_file(window, settings, of_settings);

		if (paths.empty()) {
			_label_filter_template_path->set_text("(no filter template selected)");
		}
		else {
			_label_filter_template_path->set_text(paths.front());
			_filter_template_path = std::move(paths.front());
			_inserter.push_event(events::filter_template_path_changed{});
		}
	};

	return el::share(el::htile(
		el::align_center_middle(el::hold(_label_filter_template_path)),
		make_button("browse...", on_filter_template_browse),
		make_button(el::icons::cycle, "reload", [this](bool) {
			if (!_filter_template_path)
				return;

			_inserter.push_event(events::filter_template_path_changed{});
		})
	));
}

void filter_template_state::load_filter_template(fs::log::logger& logger)
{
	if (!_filter_template_path)
		return;

	_filter_template_source = fs::utility::load_file(*_filter_template_path, logger);

	if (!_filter_template_source)
		return;

	logger.info() << "loaded filter template, " << (*_filter_template_source).size() << " bytes";

	_inserter.push_event(events::filter_template_changed{});
}

void filter_template_state::parse_filter_template(fs::generator::settings st, fs::log::logger& logger)
{
	_spirit_filter = fs::generator::sf::parse_spirit_filter(*_filter_template_source, st, logger);

	if (!_spirit_filter)
		return;

	_inserter.push_event(events::spirit_filter_changed{});
}

void filter_template_state::recompute_real_filter(const fs::lang::market::item_price_report& report, fs::log::logger& logger)
{
	if (!_spirit_filter)
		return;

	_real_filter = fs::generator::make_filter(*_spirit_filter, report.data);

	if (!_real_filter)
		return;

	logger.info() << "filter recomputed";
	_inserter.push_event(events::real_filter_changed{});
}
