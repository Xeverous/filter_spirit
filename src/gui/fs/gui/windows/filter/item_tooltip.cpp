#include <fs/gui/windows/filter/item_tooltip.hpp>
#include <fs/gui/settings/font_settings.hpp>
#include <fs/gui/auxiliary/widgets.hpp>
#include <fs/gui/auxiliary/colors.hpp>
#include <fs/lang/item.hpp>
#include <fs/lang/item_filter.hpp>

#include <imgui.h>

namespace fs::gui {

float get_item_tooltip_first_column_width(const font_settings& fonting)
{
	return gui::aux::measure_text_line(
			lang::keywords::rf::enchantment_passive_node,
			fonting.monospaced_font_size(),
			fonting.monospaced_font()
		).x + fonting.monospaced_font_size() * 2.0f;
}

constexpr auto str_not_implemented = "NOT IMPLEMENTED";

float get_item_tooltip_second_column_width(const lang::item& itm, const font_settings& fonting)
{
	const char* longest_second_column_str = str_not_implemented;
	std::size_t largest_second_column_str_size = std::strlen(str_not_implemented);
	if (itm.base_type.size() > largest_second_column_str_size) {
		longest_second_column_str = itm.base_type.c_str();
		largest_second_column_str_size = itm.base_type.size();
	}
	if (itm.class_.size() > largest_second_column_str_size) {
		longest_second_column_str = itm.class_.c_str();
		largest_second_column_str_size = itm.class_.size();
	}

	return gui::aux::measure_text_line(
			longest_second_column_str,
			fonting.monospaced_font_size(),
			fonting.monospaced_font()
		).x + fonting.monospaced_font_size() * 2.0f;
}

void draw_item_tooltip_first_column()
{
	namespace kw = lang::keywords::rf;

	ImGui::TextDisabled("Name:");
	ImGui::Text("%s:", kw::base_type);
	ImGui::Text("%s:", kw::class_);
	ImGui::Text("%s:", kw::rarity);
	ImGui::Text("%s:", kw::sockets);
	ImGui::Text("%s:", kw::item_level);
	ImGui::Text("%s:", kw::drop_level);
	ImGui::Text("%s:", kw::width);
	ImGui::Text("%s:", kw::height);
	ImGui::Text("%s:", kw::quality);
	ImGui::Text("%s:", kw::stack_size);
	ImGui::TextDisabled("MaxStackSize:");
	ImGui::Text("%s:", kw::gem_level);
	ImGui::TextDisabled("MaxGemLevel:");
	ImGui::Text("%s:", kw::map_tier);
	ImGui::Text("%s:", kw::corrupted_mods);

	ImGui::Text("%s:", kw::has_influence);
	ImGui::Text("%s:", kw::has_explicit_mod);
	ImGui::Text("%s:", kw::has_enchantment);
	ImGui::Text("%s:", kw::enchantment_passive_node);
	ImGui::TextDisabled("HasAnnointment:");

	ImGui::Text("%s:", kw::identified);
	ImGui::Text("%s:", kw::corrupted);
	ImGui::Text("%s:", kw::mirrored);
	ImGui::Text("%s:", kw::fractured_item);
	ImGui::Text("%s:", kw::synthesised_item);
	ImGui::Text("%s:", kw::shaped_map);
	ImGui::Text("%s:", kw::elder_map);
	ImGui::Text("%s:", kw::blighted_map);
	ImGui::Text("%s:", kw::replica);
	ImGui::Text("%s:", kw::transfigured_gem);
}

void draw_item_tooltip_second_column(const lang::item& itm)
{
	if (itm.name)
		ImGui::TextUnformatted((*itm.name).c_str());
	else
		ImGui::TextUnformatted("-");

	ImGui::TextUnformatted(itm.base_type.c_str());
	ImGui::TextUnformatted(itm.class_.c_str());

	{
		const auto rarity = to_string_view(itm.rarity_);
		const auto rarity_text_color =
			itm.rarity_ == lang::rarity_type::unique ? gui::aux::color_unique_item :
			itm.rarity_ == lang::rarity_type::rare   ? gui::aux::color_rare_item   :
			itm.rarity_ == lang::rarity_type::magic  ? gui::aux::color_magic_item  : gui::aux::color_normal_item;
		ImGui::PushStyleColor(ImGuiCol_Text, rarity_text_color);
		ImGui::TextUnformatted(rarity.data(), rarity.data() + rarity.size());
		ImGui::PopStyleColor();
	}

	ImGui::TextUnformatted(to_string(itm.sockets).c_str()); // TODO add colors
	ImGui::Text("%d", itm.item_level);
	ImGui::Text("%d", itm.drop_level);
	ImGui::Text("%d", itm.width);
	ImGui::Text("%d", itm.height);
	ImGui::Text("%d", itm.quality);
	ImGui::Text("%d", itm.stack_size);
	ImGui::Text("%d", itm.max_stack_size);
	ImGui::Text("%d", itm.gem_level);
	ImGui::Text("%d", itm.max_gem_level);
	ImGui::Text("%d", itm.map_tier);
	ImGui::Text("%d", itm.corrupted_mods);

	if (itm.influence.is_none()) {
		ImGui::TextUnformatted("None");
	}
	else {
		bool first_influence_text = true;
		const auto output_influence = [&](bool condition, const char* name) {
			if (!condition)
				return;

			if (!first_influence_text) {
				ImGui::SameLine(0, 0);
			}

			ImGui::Text("%s ", name);
			first_influence_text = false;
		};

		// TODO colors?
		namespace kw = lang::keywords::rf;
		output_influence(itm.influence.shaper, kw::shaper);
		output_influence(itm.influence.elder, kw::elder);
		output_influence(itm.influence.crusader, kw::crusader);
		output_influence(itm.influence.redeemer, kw::redeemer);
		output_influence(itm.influence.hunter, kw::hunter);
		output_influence(itm.influence.warlord, kw::warlord);
	}

	ImGui::TextDisabled(str_not_implemented);
	ImGui::TextDisabled(str_not_implemented);
	ImGui::TextDisabled(str_not_implemented);
	ImGui::TextDisabled(str_not_implemented);

	const auto output_boolean = [](bool value) {
		namespace kw = lang::keywords::rf;

		if (value)
			ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(gui::aux::color_true), kw::true_);
		else
			ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(gui::aux::color_false), kw::false_);
	};

	output_boolean(itm.is_identified);
	output_boolean(itm.is_corrupted());
	output_boolean(itm.is_mirrored);
	output_boolean(itm.is_fractured);
	output_boolean(itm.is_synthesised);
	output_boolean(itm.is_shaped_map);
	output_boolean(itm.is_elder_map);
	output_boolean(itm.is_blighted_map());
	output_boolean(itm.is_replica);
	output_boolean(itm.is_transfigured_gem);
}

// TODO draw also applied filter rules
void draw_item_tooltip(const lang::item& itm, const lang::item_filtering_result& /* result */, const font_settings& fonting)
{
	const auto _1 = fonting.scoped_monospaced_font();

	const auto first_column_width = get_item_tooltip_first_column_width(fonting);
	const auto second_column_width = get_item_tooltip_second_column_width(itm, fonting);

	ImGui::SetNextWindowSize(ImVec2(first_column_width + second_column_width, 0));
	gui::aux::scoped_tooltip _2;

	ImGui::Columns(2);

	ImGui::SetColumnWidth(-1, first_column_width);
	draw_item_tooltip_first_column();

	ImGui::NextColumn();
	draw_item_tooltip_second_column(itm);

	ImGui::Columns(1); // reset back to single column

	ImGui::Dummy({0.0f, ImGui::GetFontSize() * 1.0f});
	ImGui::TextUnformatted("(right-click to debug this item)");
	ImGui::TextDisabled("(filters do not support these properties)");
}

}
