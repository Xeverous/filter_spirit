#include <fs/gui/windows/filter/filter_state_mediator.hpp>
#include <fs/gui/application.hpp>
#include <fs/utility/file.hpp>

#include <utility>

namespace fs::gui {

#ifndef __EMSCRIPTEN__
void filter_state_mediator::load_source_file(std::string path)
{
	_source.load_source_file(std::move(path), *this);
}
#endif

void filter_state_mediator::open_text_input()
{
	_source.open_source_edit();
}

void filter_state_mediator::new_source(std::string source)
{
	_source.new_source(std::move(source), *this);
}

void filter_state_mediator::new_filter_representation(std::optional<lang::item_filter> representation)
{
	_filter_representation = std::move(representation);
	on_filter_representation_change(filter_representation());
}

void filter_state_mediator::on_filter_representation_change(const lang::item_filter* filter_representation)
{
	if (filter_representation)
		_loot_state.refilter_items(*filter_representation, *this);
	else
		_loot_state.clear_filter_results(*this);
}

void filter_state_mediator::on_debug_open(const lang::item& itm, const lang::item_filtering_result& result)
{
	const parser::lookup_data* lookup = lookup_data();
	if (lookup == nullptr)
		return;

	const parser::line_lookup* lines = line_lookup();
	if (lines == nullptr)
		return;

	_debug_state.open_debug(*lookup, *lines, itm, result);
}

void filter_state_mediator::on_loot_change()
{
	_debug_state.invalidate();
}

void filter_state_mediator::on_filter_results_change()
{
	const parser::lookup_data* lookup = lookup_data();
	const parser::line_lookup* lines = line_lookup();

	if (lookup && lines)
		_debug_state.recompute(*lookup, *lines);
	else
		_debug_state.invalidate();
}

void filter_state_mediator::on_filter_results_clear()
{
	_debug_state.invalidate();
}

void filter_state_mediator::draw_interface(application& app)
{
	ImGui::Columns(2, nullptr, false);

	_source.draw_interface(app.font_settings(), *this);
	draw_interface_derived(app);
	draw_interface_filter_representation();
	draw_interface_logs(app);

	ImGui::NextColumn();

	draw_interface_loot(app);
	_debug_state.draw_interface(app.font_settings(), lookup_data(), line_lookup());
}

void filter_state_mediator::draw_interface_filter_representation()
{
	if (ImGui::CollapsingHeader("Filter representation", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_filter_representation) {
			ImGui::TextWrapped("Ready, %zu blocks", (*_filter_representation).blocks.size());
			draw_interface_save_filter(*_filter_representation, logger());
		}
		else {
			ImGui::TextWrapped("Not available.");
		}
	}
}

void filter_state_mediator::draw_interface_logs(application& app)
{
	if (ImGui::CollapsingHeader("Logs", ImGuiTreeNodeFlags_DefaultOpen)) {
		draw_interface_logs_derived(_logger, app.font_settings());
	}
}

void filter_state_mediator::draw_interface_loot(application& app)
{
	std::array<char, 64> loot_status_str_buf;
	if (const auto& str = "Loot preview & filter debug"; _loot_state.num_items() > 0u) {
		std::snprintf(
			loot_status_str_buf.data(),
			loot_status_str_buf.size(),
			"%s (%zu items, %zu hidden)",
			str,
			_loot_state.num_items(),
			_loot_state.num_hidden_items());
	}
	else {
		constexpr auto str_size = std::size(str);
		constexpr auto buf_size = loot_status_str_buf.size();
		static_assert(buf_size >= str_size);
		std::memcpy(loot_status_str_buf.data(), str, str_size);
	}

	if (ImGui::CollapsingHeader(loot_status_str_buf.data(), ImGuiTreeNodeFlags_DefaultOpen)) {
		if (!_filter_representation) {
			ImGui::TextWrapped("No filter representation available.");
			return;
		}

		FS_ASSERT(_source.source() != nullptr);

		_loot_state.update_items(*_filter_representation);
		_loot_state.draw_interface(app, *this);
	}
}

}
