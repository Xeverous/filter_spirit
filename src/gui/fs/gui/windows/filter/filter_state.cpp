#include <fs/gui/windows/filter/filter_state.hpp>
#include <fs/gui/ui_utils.hpp>
#include <fs/parser/parser.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/generator/make_item_filter.hpp>

#include <imgui.h>

#include <utility>

namespace fs::gui {

namespace detail {

void filter_state_base::new_source(std::optional<std::string> source, log::logger& logger)
{
	_source = std::move(source);
	on_source_change(_source, logger);
}

void filter_state_base::new_filter_representation(std::optional<lang::item_filter> filter_representation, log::logger& logger)
{
	_filter_representation = std::move(filter_representation);
	on_filter_representation_change(_filter_representation, logger);
}

void filter_state_base::on_filter_representation_change(const std::optional<lang::item_filter>& filter_representation, log::logger& /* logger */)
{
	if (filter_representation)
		_loot_state.refilter_items(*filter_representation);
	else
		_loot_state.clear_filter_results();
}

void filter_state_base::draw_interface(application& app)
{
	ImGui::Columns(2, nullptr, false);

	draw_interface_source();
	draw_interface_derived();
	draw_interface_filter_representation();

	ImGui::NextColumn();

	draw_interface_loot(app);
}

void filter_state_base::draw_interface_source()
{
	if (ImGui::CollapsingHeader("Source", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_source)
			ImGui::TextWrapped("Source loaded, %zu bytes", (*_source).size());
		else
			ImGui::TextWrapped("Source not loaded.");
	}
}

void filter_state_base::draw_interface_filter_representation()
{
	if (ImGui::CollapsingHeader("Filter representation", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_filter_representation)
			ImGui::TextWrapped("Ready, %zu blocks", (*_filter_representation).blocks.size());
		else
			ImGui::TextWrapped("Not available.");
	}
}

void filter_state_base::draw_interface_loot(application& app)
{
	if (ImGui::CollapsingHeader("Loot preview & filter debug", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (!_filter_representation) {
			ImGui::TextWrapped("No filter representation available.");
			return;
		}

		_loot_state.draw_interface(app);
	}
}

} // namespace detail

void real_filter_state::on_source_change(const std::optional<std::string>& source, log::logger& logger)
{
	if (!source) {
		new_parsed_real_filter(std::nullopt, logger);
		return;
	}

	std::variant<parser::parsed_real_filter, parser::parse_failure_data> result = parser::parse_real_filter(*source);

	if (std::holds_alternative<parser::parse_failure_data>(result)) {
		parser::print_parse_errors(std::get<parser::parse_failure_data>(result), logger);
		_parsed_real_filter = std::nullopt;
		new_parsed_real_filter(std::nullopt, logger);
	}
	else {
		new_parsed_real_filter(std::move(std::get<parser::parsed_real_filter>(result)), logger);
	}
}

void real_filter_state::new_parsed_real_filter(std::optional<parser::parsed_real_filter> parsed_real_filter, log::logger& logger)
{
	_parsed_real_filter = std::move(parsed_real_filter);
	on_parsed_real_filter_change(_parsed_real_filter, logger);
}

void real_filter_state::on_parsed_real_filter_change(const std::optional<parser::parsed_real_filter>& parsed_real_filter, log::logger& logger)
{
	if (!parsed_real_filter) {
		new_filter_representation(std::nullopt, logger);
		return;
	}

	compiler::outcome<lang::item_filter> result = compiler::compile_real_filter({}, (*_parsed_real_filter).ast);
	compiler::output_logs(result.logs(), (*_parsed_real_filter).lookup, logger);

	if (result.has_result())
		new_filter_representation(std::move(result.result()), logger);
	else
		new_filter_representation(std::nullopt, logger);
}

void real_filter_state::draw_interface_derived()
{
	if (ImGui::CollapsingHeader("Real filter abstract syntax tree", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_parsed_real_filter) {
			ImGui::TextWrapped(
				"Parse successful, %zu blocks, %zu recorded AST nodes",
				(*_parsed_real_filter).ast.size(),
				(*_parsed_real_filter).lookup.size());
		}
		else {
			ImGui::TextUnformatted("Not available.");
		}
	}
}

void spirit_filter_state::on_source_change(const std::optional<std::string>& source, log::logger& logger)
{
	if (!source) {
		new_parsed_spirit_filter(std::nullopt, logger);
		return;
	}

	std::variant<parser::parsed_spirit_filter, parser::parse_failure_data> result = parser::parse_spirit_filter(*source);

	if (std::holds_alternative<parser::parse_failure_data>(result)) {
		parser::print_parse_errors(std::get<parser::parse_failure_data>(result), logger);
		new_parsed_spirit_filter(std::nullopt, logger);
	}
	else {
		new_parsed_spirit_filter(std::move(std::get<parser::parsed_spirit_filter>(result)), logger);
	}
}

void spirit_filter_state::new_parsed_spirit_filter(std::optional<parser::parsed_spirit_filter> parsed_spirit_filter, log::logger& logger)
{
	_parsed_spirit_filter = std::move(parsed_spirit_filter);
	on_parsed_spirit_filter_change(_parsed_spirit_filter, logger);
}

void spirit_filter_state::on_parsed_spirit_filter_change(const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter, log::logger& logger)
{
	if (!parsed_spirit_filter) {
		new_spirit_filter_symbols(std::nullopt, logger);
		return;
	}

	compiler::outcome<lang::symbol_table> result = compiler::resolve_spirit_filter_symbols(
		{}, (*parsed_spirit_filter).ast.definitions);

	compiler::output_logs(result.logs(), (*_parsed_spirit_filter).lookup, logger);

	if (result.has_result())
		new_spirit_filter_symbols(std::move(result.result()), logger);
	else
		new_spirit_filter_symbols(std::nullopt, logger);
}

void spirit_filter_state::new_spirit_filter_symbols(std::optional<lang::symbol_table> spirit_filter_symbols, log::logger& logger)
{
	_spirit_filter_symbols = std::move(spirit_filter_symbols);
	on_spirit_filter_symbols_change(_parsed_spirit_filter, _spirit_filter_symbols, logger);
}

void spirit_filter_state::on_spirit_filter_symbols_change(
	const std::optional<parser::parsed_spirit_filter>& parsed_spirit_filter,
	const std::optional<lang::symbol_table>& spirit_filter_symbols,
	log::logger& logger)
{
	if (!spirit_filter_symbols || !parsed_spirit_filter) {
		new_spirit_filter(std::nullopt, logger);
		return;
	}

	compiler::outcome<lang::spirit_item_filter> result = compiler::compile_spirit_filter_statements(
		{}, (*parsed_spirit_filter).ast.statements, *spirit_filter_symbols);

	compiler::output_logs(result.logs(), (*parsed_spirit_filter).lookup, logger);

	if (result.has_result())
		new_spirit_filter(std::move(result.result()), logger);
	else
		new_spirit_filter(std::nullopt, logger);
}

void spirit_filter_state::new_spirit_filter(std::optional<lang::spirit_item_filter> spirit_filter, log::logger& logger)
{
	_spirit_filter = std::move(spirit_filter);
	on_spirit_filter_change(_spirit_filter, logger);
}

void spirit_filter_state::on_spirit_filter_change(const std::optional<lang::spirit_item_filter>& spirit_filter, log::logger& logger)
{
	if (!spirit_filter) {
		new_filter_representation(std::nullopt, logger);
		return;
	}

	new_filter_representation(generator::make_item_filter(*_spirit_filter, {}), logger);
}

void spirit_filter_state::draw_interface_derived()
{
	draw_interface_parsed_spirit_filter();
	draw_interface_spirit_filter_symbols();
	draw_interface_spirit_filter();
}

void spirit_filter_state::draw_interface_parsed_spirit_filter()
{
	if (ImGui::CollapsingHeader("Spirit filter abstract syntax tree", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_parsed_spirit_filter) {
			ImGui::TextWrapped(
				"Parse successful, %zu definitions, %zu root level blocks, %zu recorded AST nodes",
				(*_parsed_spirit_filter).ast.definitions.size(),
				(*_parsed_spirit_filter).ast.statements.size(),
				(*_parsed_spirit_filter).lookup.size());
		}
		else {
			ImGui::TextUnformatted("Not available.");
		}
	}
}

void spirit_filter_state::draw_interface_spirit_filter_symbols()
{
	if (ImGui::CollapsingHeader("Spirit filter definitions", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_spirit_filter_symbols) {
			ImGui::TextWrapped("Resolving definitions successful, %zu definitions", (*_spirit_filter_symbols).size());
		}
		else {
			ImGui::TextWrapped("Not available.");
		}
	}
}

void spirit_filter_state::draw_interface_spirit_filter()
{
	if (ImGui::CollapsingHeader("Spirit filter representation", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_spirit_filter) {
			ImGui::TextWrapped("Building representation successful, %zu total blocks", (*_spirit_filter).blocks.size());
		}
		else {
			ImGui::TextWrapped("Not available.");
		}
	}
}

}
