#include <fs/gui/windows/filter/real_filter_state_mediator.hpp>
#include <fs/compiler/compiler.hpp>

namespace fs::gui {

const parser::lookup_data* real_filter_state_mediator::lookup_data() const
{
	if (_parsed_real_filter)
		return &((*_parsed_real_filter).lookup);
	else
		return nullptr;
}

const parser::line_lookup* real_filter_state_mediator::line_lookup() const
{
	if (_parsed_real_filter)
		return &((*_parsed_real_filter).lines);
	else
		return nullptr;
}

void real_filter_state_mediator::on_source_change(const std::string* source)
{
	if (!source) {
		new_parsed_real_filter(std::nullopt);
		return;
	}

	std::variant<parser::parsed_real_filter, parser::parse_failure_data> result = parser::parse_real_filter(*source);

	if (std::holds_alternative<parser::parse_failure_data>(result)) {
		parser::print_parse_errors(std::get<parser::parse_failure_data>(result), logger());
		new_parsed_real_filter(std::nullopt);
	}
	else {
		new_parsed_real_filter(std::move(std::get<parser::parsed_real_filter>(result)));
	}
}

void real_filter_state_mediator::new_parsed_real_filter(std::optional<parser::parsed_real_filter> filter)
{
	_parsed_real_filter = std::move(filter);
	on_parsed_real_filter_change(parsed_real_filter());
}

void real_filter_state_mediator::on_parsed_real_filter_change(const parser::parsed_real_filter* parsed_real_filter)
{
	if (!parsed_real_filter) {
		new_filter_representation(std::nullopt);
		return;
	}

	compiler::outcome<lang::item_filter> result = compiler::compile_real_filter({}, parsed_real_filter->ast);
	compiler::output_logs(result.logs(), parsed_real_filter->lookup, logger());

	if (result.has_result())
		new_filter_representation(std::move(result.result()));
	else
		new_filter_representation(std::nullopt);
}

void real_filter_state_mediator::draw_interface_derived(application& /* app */)
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

void real_filter_state_mediator::draw_interface_logs_derived(gui_logger& gl, const fonting& f)
{
	gl.draw(f, _logger);
}

}
