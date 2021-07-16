#include <fs/gui/windows/filter/spirit_filter_state_mediator.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/generator/make_item_filter.hpp>
#include <fs/generator/generator.hpp>
#include <fs/utility/assert.hpp>
#include <fs/utility/monadic.hpp>
#include <fs/utility/file.hpp>

#include <imgui.h>

#ifndef __EMSCRIPTEN__
#include <tinyfiledialogs.h>
#endif

namespace fs::gui {

namespace {

// Note: this implementation will not compile on Emscripten,
// tiny file dialogs do not work there
void save_filter(const lang::market::item_price_metadata& metadata, const lang::item_filter& filter, log::logger& logger)
{
	const char* const path = tinyfd_saveFileDialog("Save filter", nullptr, 0, nullptr, nullptr);
	if (path == nullptr)
		return;

	std::string output_content = generator::item_filter_to_string(filter);
	generator::prepend_metadata(metadata, output_content);

	std::filesystem::path output_filepath(path);
	if (utility::save_file(output_filepath, output_content, logger))
		logger.info() << "Item filter successfully saved as " << output_filepath.generic_string() << ".\n";
}

}

log::logger& spirit_filter_state_mediator::logger()
{
	FS_ASSERT(_logger_ptr != nullptr);
	return *_logger_ptr;
}

void spirit_filter_state_mediator::clear_logs()
{
	FS_ASSERT(_logger_ptr != nullptr);
	(*_logger_ptr)([&](log::buffer_logger& l){ l.clear(); });
}

const parser::parse_metadata* spirit_filter_state_mediator::parse_metadata() const
{
	if (_parsed_spirit_filter)
		return &((*_parsed_spirit_filter).metadata);
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
		logger().info() << "Parse successful.";
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

	compiler::diagnostics_container diagnostics;
	boost::optional<compiler::symbol_table> result = compiler::resolve_spirit_filter_symbols(
		{}, parsed_spirit_filter->ast.definitions, diagnostics);

	compiler::output_diagnostics(diagnostics, parsed_spirit_filter->metadata, logger());

	if (result)
		logger().info() << "Resolving definitions successful.";

	new_spirit_filter_symbols(utility::to_std_optional(std::move(result)));
}

void spirit_filter_state_mediator::new_spirit_filter_symbols(std::optional<compiler::symbol_table> symbols)
{
	_spirit_filter_symbols = std::move(symbols);
	on_spirit_filter_symbols_change(parsed_spirit_filter(), spirit_filter_symbols());
}

void spirit_filter_state_mediator::on_spirit_filter_symbols_change(
	const parser::parsed_spirit_filter* parsed_spirit_filter,
	const compiler::symbol_table* spirit_filter_symbols)
{
	if (!spirit_filter_symbols || !parsed_spirit_filter) {
		new_spirit_filter(std::nullopt);
		return;
	}

	compiler::diagnostics_container diagnostics;
	boost::optional<lang::spirit_item_filter> result = compiler::compile_spirit_filter_statements(
		{}, parsed_spirit_filter->ast.statements, *spirit_filter_symbols, diagnostics);

	compiler::output_diagnostics(diagnostics, parsed_spirit_filter->metadata, logger());

	if (result)
		logger().info() << "Compilation successful.";

	new_spirit_filter(utility::to_std_optional(std::move(result)));
}

void spirit_filter_state_mediator::new_spirit_filter(std::optional<lang::spirit_item_filter> filter)
{
	_spirit_filter = std::move(filter);
	on_spirit_filter_change(spirit_filter());
}

void spirit_filter_state_mediator::on_spirit_filter_change(const lang::spirit_item_filter* spirit_filter)
{
	refresh_filter_representation(spirit_filter, price_report().data);
}

void spirit_filter_state_mediator::on_price_report_change(const lang::market::item_price_report& report)
{
	refresh_filter_representation(spirit_filter(), report.data);
}

void spirit_filter_state_mediator::refresh_filter_representation(
	const lang::spirit_item_filter* spirit_filter,
	const lang::market::item_price_data& item_price_data)
{
	if (!spirit_filter) {
		new_filter_representation(std::nullopt);
		return;
	}

	new_filter_representation(generator::make_item_filter(*spirit_filter, item_price_data));
}

void spirit_filter_state_mediator::draw_interface_derived(const network_settings& networking, network::cache& cache)
{
	draw_interface_parsed_spirit_filter();
	draw_interface_spirit_filter_symbols();
	draw_interface_spirit_filter();
	_market_data.draw_interface(networking, cache, *this);
}

void spirit_filter_state_mediator::draw_interface_save_filter(const lang::item_filter& filter, log::logger& logger)
{
	if (ImGui::Button("Save filter as..."))
		save_filter(_market_data.price_report().metadata, filter, logger);
}

void spirit_filter_state_mediator::draw_interface_logs_derived(gui_logger& gl, const font_settings& fonting)
{
	FS_ASSERT(_logger_ptr != nullptr);
	(*_logger_ptr)([&](log::buffer_logger& bl) {
		gl.draw(fonting, bl);
	});
}

void spirit_filter_state_mediator::draw_interface_parsed_spirit_filter()
{
	if (ImGui::CollapsingHeader("Spirit filter abstract syntax tree", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (_parsed_spirit_filter) {
			ImGui::TextWrapped(
				"Parse successful, %zu definitions, %zu root level blocks, %zu recorded AST nodes",
				(*_parsed_spirit_filter).ast.definitions.size(),
				(*_parsed_spirit_filter).ast.statements.size(),
				(*_parsed_spirit_filter).metadata.lookup.size());
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
