#include <fs/gui/windows/filter/filter_state_mediator.hpp>
#include <fs/utility/file.hpp>

#include <utility>

namespace fs::gui {

filter_state_mediator::filter_state_mediator(const fonting& f)
: _logger(f)
{
}

void filter_state_mediator::load_source_file(std::string path)
{
	_source.load_source_file(std::move(path), *this);
}

void filter_state_mediator::new_filter_representation(std::optional<lang::item_filter> filter_representation)
{
	_filter_representation = std::move(filter_representation);
	on_filter_representation_change(_filter_representation);
}

void filter_state_mediator::on_filter_representation_change(const std::optional<lang::item_filter>& filter_representation)
{
	if (filter_representation)
		_loot_state.refilter_items(*filter_representation);
	else
		_loot_state.clear_filter_results();
}

void filter_state_mediator::draw_interface(application& app)
{
	ImGui::Columns(2, nullptr, false);

	_source.draw_interface(*this);
	draw_interface_derived();
	draw_interface_filter_representation();
	draw_interface_logs();

	ImGui::NextColumn();

	draw_interface_loot(app);
}

void filter_state_mediator::draw_interface_filter_representation()
{
	if (ImGui::CollapsingHeader("Filter representation", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_filter_representation)
			ImGui::TextWrapped("Ready, %zu blocks", (*_filter_representation).blocks.size());
		else
			ImGui::TextWrapped("Not available.");
	}
}

void filter_state_mediator::draw_interface_logs()
{
	if (ImGui::CollapsingHeader("Logs", ImGuiTreeNodeFlags_DefaultOpen)) {
		_logger.draw();
	}
}

void filter_state_mediator::draw_interface_loot(application& app)
{
	std::array<char, 48> loot_status_str_buf;
	if (const auto& str = "Loot preview & filter debug"; _loot_state.num_items() > 0u) {
		std::snprintf(
			loot_status_str_buf.data(),
			loot_status_str_buf.size(),
			"%s (%zu items)",
			str,
			_loot_state.num_items());
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

		FS_ASSERT(_source.source().has_value());

		_loot_state.update_items(*_filter_representation);
		_loot_state.draw_interface(*_source.source(), app);
	}
}

}
