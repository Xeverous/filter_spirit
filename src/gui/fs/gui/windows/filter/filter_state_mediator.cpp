#include <fs/gui/windows/filter/filter_state_mediator.hpp>
#include <fs/gui/gui_settings.hpp>
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
	const parser::parse_metadata* metadata = parse_metadata();
	if (metadata == nullptr)
		return;

	_debug_state.open_debug(*metadata, itm, result);
}

void filter_state_mediator::on_loot_change()
{
	_debug_state.invalidate();
}

void filter_state_mediator::on_filter_results_change()
{
	const parser::parse_metadata* metadata = parse_metadata();

	if (metadata)
		_debug_state.recompute(*metadata);
	else
		_debug_state.invalidate();
}

void filter_state_mediator::on_filter_results_clear()
{
	_debug_state.invalidate();
}

void filter_state_mediator::draw(
	const gui_settings& settings,
	const lang::loot::item_database& db,
	lang::loot::generator& gen,
	network::cache& network_cache)
{
	ImGui::Columns(2, nullptr, false);

	_source.draw_interface(settings.fonting, *this);
	draw_interface_derived(settings.networking, network_cache);
	draw_interface_filter_representation();
	draw_interface_logs(settings.fonting);

	ImGui::NextColumn();

	draw_interface_loot(settings.fonting, db, gen);
	_debug_state.draw_interface(settings.fonting, parse_metadata());
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

void filter_state_mediator::draw_interface_logs(const font_settings& fonting)
{
	if (ImGui::CollapsingHeader("Logs", ImGuiTreeNodeFlags_DefaultOpen)) {
		draw_interface_logs_derived(_logger, fonting);
	}
}

void filter_state_mediator::draw_interface_loot(
	const font_settings& fonting,
	const lang::loot::item_database& db,
	lang::loot::generator& gen)
{
	std::array<char, 64> loot_status_str_buf;
	std::snprintf(
		loot_status_str_buf.data(),
		loot_status_str_buf.size(),
		"Loot preview & filter debug (%zu items, %zu hidden)",
		_loot_state.num_items(),
		_loot_state.num_hidden_items());

	if (ImGui::CollapsingHeader(loot_status_str_buf.data(), ImGuiTreeNodeFlags_DefaultOpen)) {
		if (!_filter_representation) {
			ImGui::TextWrapped("No filter representation available.");
			return;
		}

		FS_ASSERT(_source.source() != nullptr);

		_loot_state.update_items(*_filter_representation);
		_loot_state.draw_interface(fonting, db, gen, *this);
	}
}

}
