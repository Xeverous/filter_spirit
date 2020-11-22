#include <fs/gui/windows/filter/real_filter_state_mediator.hpp>
#include <fs/compiler/compiler.hpp>

namespace fs::gui {

void real_filter_state_mediator::on_source_change(const std::optional<std::string>& source)
{
	if (!source) {
		new_parsed_real_filter(std::nullopt);
		return;
	}

	std::variant<parser::parsed_real_filter, parser::parse_failure_data> result = parser::parse_real_filter(*source);

	if (std::holds_alternative<parser::parse_failure_data>(result)) {
		parser::print_parse_errors(std::get<parser::parse_failure_data>(result), logger());
		_parsed_real_filter = std::nullopt;
		new_parsed_real_filter(std::nullopt);
	}
	else {
		new_parsed_real_filter(std::move(std::get<parser::parsed_real_filter>(result)));
	}
}

void real_filter_state_mediator::new_parsed_real_filter(std::optional<parser::parsed_real_filter> parsed_real_filter)
{
	_parsed_real_filter = std::move(parsed_real_filter);
	on_parsed_real_filter_change(_parsed_real_filter);
}

void real_filter_state_mediator::on_parsed_real_filter_change(const std::optional<parser::parsed_real_filter>& parsed_real_filter)
{
	if (!parsed_real_filter) {
		new_filter_representation(std::nullopt);
		return;
	}

	compiler::outcome<lang::item_filter> result = compiler::compile_real_filter({}, (*_parsed_real_filter).ast);
	compiler::output_logs(result.logs(), (*_parsed_real_filter).lookup, logger());

	if (result.has_result())
		new_filter_representation(std::move(result.result()));
	else
		new_filter_representation(std::nullopt);
}

void real_filter_state_mediator::draw_interface_derived()
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

}
