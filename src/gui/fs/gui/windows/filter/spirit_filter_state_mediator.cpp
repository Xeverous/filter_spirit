#include <fs/gui/windows/filter/spirit_filter_state_mediator.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/generator/make_item_filter.hpp>

#include <imgui.h>

namespace fs::gui {

const parser::lookup_data* spirit_filter_state_mediator::lookup_data() const
{
	if (_parsed_spirit_filter)
		return &((*_parsed_spirit_filter).lookup);
	else
		return nullptr;
}

const parser::line_lookup* spirit_filter_state_mediator::line_lookup() const
{
	if (_parsed_spirit_filter)
		return &((*_parsed_spirit_filter).lines);
	else
		return nullptr;
}

void spirit_filter_state_mediator::on_source_change(const std::string* source)
{
	if (!source) {
		new_parsed_spirit_filter(std::nullopt);
		return;
	}

	std::variant<parser::parsed_spirit_filter, parser::parse_failure_data> result = parser::parse_spirit_filter(*source);

	if (std::holds_alternative<parser::parse_failure_data>(result)) {
		parser::print_parse_errors(std::get<parser::parse_failure_data>(result), logger());
		new_parsed_spirit_filter(std::nullopt);
	}
	else {
		new_parsed_spirit_filter(std::move(std::get<parser::parsed_spirit_filter>(result)));
	}
}

void spirit_filter_state_mediator::new_parsed_spirit_filter(std::optional<parser::parsed_spirit_filter> filter)
{
	_parsed_spirit_filter = std::move(filter);
	on_parsed_spirit_filter_change(parsed_spirit_filter());
}

void spirit_filter_state_mediator::on_parsed_spirit_filter_change(const parser::parsed_spirit_filter* parsed_spirit_filter)
{
	if (!parsed_spirit_filter) {
		new_spirit_filter_symbols(std::nullopt);
		return;
	}

	compiler::outcome<lang::symbol_table> result = compiler::resolve_spirit_filter_symbols(
		{}, parsed_spirit_filter->ast.definitions);

	compiler::output_logs(result.logs(), parsed_spirit_filter->lookup, logger());

	if (result.has_result())
		new_spirit_filter_symbols(std::move(result.result()));
	else
		new_spirit_filter_symbols(std::nullopt);
}

void spirit_filter_state_mediator::new_spirit_filter_symbols(std::optional<lang::symbol_table> symbols)
{
	_spirit_filter_symbols = std::move(symbols);
	on_spirit_filter_symbols_change(parsed_spirit_filter(), spirit_filter_symbols());
}

void spirit_filter_state_mediator::on_spirit_filter_symbols_change(
	const parser::parsed_spirit_filter* parsed_spirit_filter,
	const lang::symbol_table* spirit_filter_symbols)
{
	if (!spirit_filter_symbols || !parsed_spirit_filter) {
		new_spirit_filter(std::nullopt);
		return;
	}

	compiler::outcome<lang::spirit_item_filter> result = compiler::compile_spirit_filter_statements(
		{}, parsed_spirit_filter->ast.statements, *spirit_filter_symbols);

	compiler::output_logs(result.logs(), parsed_spirit_filter->lookup, logger());

	if (result.has_result())
		new_spirit_filter(std::move(result.result()));
	else
		new_spirit_filter(std::nullopt);
}

void spirit_filter_state_mediator::new_spirit_filter(std::optional<lang::spirit_item_filter> filter)
{
	_spirit_filter = std::move(filter);
	on_spirit_filter_change(spirit_filter());
}

void spirit_filter_state_mediator::on_spirit_filter_change(const lang::spirit_item_filter* spirit_filter)
{
	if (!spirit_filter) {
		new_filter_representation(std::nullopt);
		return;
	}

	new_filter_representation(generator::make_item_filter(*spirit_filter, {}));
}

void spirit_filter_state_mediator::draw_interface_derived()
{
	draw_interface_parsed_spirit_filter();
	draw_interface_spirit_filter_symbols();
	draw_interface_spirit_filter();
}

void spirit_filter_state_mediator::draw_interface_parsed_spirit_filter()
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

void spirit_filter_state_mediator::draw_interface_spirit_filter_symbols()
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

void spirit_filter_state_mediator::draw_interface_spirit_filter()
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
