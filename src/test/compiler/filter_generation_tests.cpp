#include "common/test_fixtures.hpp"
#include "common/string_operations.hpp"

#include <fs/compiler/compiler.hpp>
#include <fs/log/string_logger.hpp>
#include <fs/lang/market/item_price_data.hpp>
#include <fs/utility/file.hpp>

#include <boost/test/unit_test.hpp>

#include <string>
#include <string_view>
#include <stdexcept>
#include <filesystem>

namespace ut = boost::unit_test;

namespace fs::test {
namespace {

std::string generate_filter(
	std::string_view input,
	compiler::settings st = {},
	const lang::market::item_price_data& ipd = {})
{
	log::string_logger logger;
	std::optional<std::string> filter = compiler::parse_compile_generate_spirit_filter_without_preamble(
		input, ipd, st, logger);
	BOOST_TEST_REQUIRE(filter.has_value(), "test written incorrectly: filter generation failed:\n" << logger.str());
	return *filter;
}

boost::test_tools::predicate_result compile_from_files(
	std::filesystem::path input_path,
	std::filesystem::path expected_output_path,
	compiler::settings st = {},
	const lang::market::item_price_data& ipd = {})
{
	const auto load_file = [](std::filesystem::path p) {
		std::error_code ec{};
		const std::string file = utility::load_file(p, ec);

		if (ec)
			throw std::runtime_error(
				ec.message() + ":\n"
				"Can not open \"" + p.generic_string() + "\"\n"
				"Make sure that:\n"
				"- the test executable is run in its own directory\n"
				"- test files are in correct place\n"
				"- paths specified in test are correct\n");

		return file;
	};

	const std::filesystem::path test_dir = "test_files/";
	const std::string input = load_file(test_dir / input_path);
	const std::string expected_output = load_file(test_dir / expected_output_path);

	// TODO: if prefix is "common", run in both poe1 and poe2 modes
	return compare_strings(generate_filter(input, st, ipd), expected_output);
}

// convenience overload - pass only prefix name, extensions will be added automatically
boost::test_tools::predicate_result compile_from_files(
	std::filesystem::path input_output_prefix_path,
	compiler::settings st = {},
	const lang::market::item_price_data& ipd = {})
{
	// copy path objects because all of concat/opeator+=/operator/= modify in-place
	const auto input_path = std::filesystem::path(input_output_prefix_path)
		.concat(lang::constants::file_extension_filter_template);

	const auto output_path = std::filesystem::path(input_output_prefix_path)
		.concat(st.ruthless_mode ?
			lang::constants::file_extension_filter_ruthless :
			lang::constants::file_extension_filter);

	return compile_from_files(input_path, output_path, st, ipd);
}

}

BOOST_AUTO_TEST_SUITE(compiler_suite)

BOOST_AUTO_TEST_CASE(minimal_input_generate_filter)
{
	BOOST_TEST(compile_from_files("common/empty"));
}

BOOST_AUTO_TEST_SUITE(
	compiler_filter_generation_suite,
	* ut::depends_on("compiler_suite/minimal_input_generate_filter"))

BOOST_AUTO_TEST_CASE(comments)
{
	BOOST_TEST(compile_from_files("common/comments"));
}

BOOST_AUTO_TEST_CASE(single_action)
{
	BOOST_TEST(compile_from_files("common/single_action"));
}

BOOST_AUTO_TEST_CASE(single_condition)
{
	BOOST_TEST(compile_from_files("common/single_condition"));
}

BOOST_AUTO_TEST_CASE(multiple_actions)
{
	BOOST_TEST(compile_from_files("common/multiple_actions"));
}

BOOST_AUTO_TEST_CASE(multiple_conditions)
{
	BOOST_TEST(compile_from_files("poe1/multiple_conditions"));
}

BOOST_AUTO_TEST_CASE(range_condition)
{
	BOOST_TEST(compile_from_files("common/range_condition"));
}

BOOST_AUTO_TEST_CASE(range_condition_min_max)
{
	BOOST_TEST(compile_from_files("common/range_condition_min_max"));
}

BOOST_AUTO_TEST_CASE(strings_condition)
{
	BOOST_TEST(compile_from_files("poe1/strings_condition"));
}

BOOST_AUTO_TEST_CASE(influences_condition)
{
	BOOST_TEST(compile_from_files("poe1/influences_condition"));
}

BOOST_AUTO_TEST_CASE(socket_spec_conditions)
{
	BOOST_TEST(compile_from_files("poe1/socket_spec_conditions"));
}

BOOST_AUTO_TEST_CASE(constants)
{
	BOOST_TEST(compile_from_files("common/constants"));
}

BOOST_AUTO_TEST_CASE(expand_actions)
{
	BOOST_TEST(compile_from_files("common/expand_actions"));
}

BOOST_AUTO_TEST_CASE(expand_in_expand)
{
	BOOST_TEST(compile_from_files("common/expand_in_expand"));
}

BOOST_AUTO_TEST_CASE(expand_in_expand_override)
{
	BOOST_TEST(compile_from_files("common/expand_in_expand_override"));
}

BOOST_AUTO_TEST_CASE(expand_nested_tree)
{
	BOOST_TEST(compile_from_files("poe1/expand_nested_tree"));
}

BOOST_AUTO_TEST_CASE(nested_actions)
{
	BOOST_TEST(compile_from_files("common/nested_actions"));
}

BOOST_AUTO_TEST_CASE(nested_blocks)
{
	BOOST_TEST(compile_from_files("common/nested_blocks"));
}

BOOST_AUTO_TEST_CASE(action_override)
{
	BOOST_TEST(compile_from_files("common/action_override"));
}

BOOST_AUTO_TEST_CASE(dynamic_visibility_enabled)
{
	BOOST_TEST(compile_from_files("common/dynamic_visibility_enabled"));
}

BOOST_AUTO_TEST_CASE(dynamic_visibility_disabled)
{
	// TODO use the same input file as dynamic_visibility_enabled and use configuration feature
	BOOST_TEST(compile_from_files("common/dynamic_visibility_disabled"));
}

BOOST_AUTO_TEST_CASE(dynamic_visibility_multiple_values)
{
	BOOST_TEST(compile_from_files("common/dynamic_visibility_multiple_values"));
}

BOOST_AUTO_TEST_CASE(dynamic_visibility_ruthless)
{
	compiler::settings st;
	st.ruthless_mode = true;
	BOOST_TEST(compile_from_files("poe1/dynamic_visibility_ruthless", st));
}

BOOST_AUTO_TEST_CASE(continue_statement)
{
	BOOST_TEST(compile_from_files("common/continue_statement"));
}

BOOST_AUTO_TEST_CASE(import_statement)
{
	BOOST_TEST(compile_from_files("common/import_statement"));
}

BOOST_AUTO_TEST_CASE(disabled_actions)
{
	BOOST_TEST(compile_from_files("common/disabled_actions"));
}

BOOST_AUTO_TEST_CASE(disabled_sounds)
{
	BOOST_TEST(compile_from_files("common/disabled_sounds"));
}

BOOST_AUTO_TEST_CASE(delirium_new_colors_and_shapes)
{
	BOOST_TEST(compile_from_files("common/delirium_new_colors_and_shapes"));
}

BOOST_AUTO_TEST_CASE(heist_new_replica_and_alternate_quality_conditions)
{
	BOOST_TEST(compile_from_files("common/heist_new_replica_and_alternate_quality_conditions"));
}

BOOST_AUTO_TEST_CASE(ritual_new_ranged_string_array_conditions)
{
	BOOST_TEST(compile_from_files("common/ritual_new_ranged_string_array_conditions"));
}

BOOST_AUTO_TEST_CASE(scourge_new_conditions)
{
	BOOST_TEST(compile_from_files("poe1/scourge_new_conditions"));
}

BOOST_AUTO_TEST_CASE(crucible_and_affliction_new_conditions)
{
	BOOST_TEST(compile_from_files("poe1/crucible_and_affliction_new_conditions"));
}

BOOST_AUTO_TEST_CASE(siege_of_the_atlas_new_conditions)
{
	BOOST_TEST(compile_from_files("poe1/siege_of_the_atlas_new_conditions"));
}

BOOST_AUTO_TEST_CASE(trarthus_new_conditions)
{
	BOOST_TEST(compile_from_files("poe1/trarthus_new_conditions"));
}

BOOST_AUTO_TEST_CASE(shaper_voice_lines)
{
	BOOST_TEST(compile_from_files("common/shaper_voice_lines"));
}

BOOST_AUTO_TEST_CASE(custom_alert_sound)
{
	BOOST_TEST(compile_from_files("common/custom_alert_sound"));
}

BOOST_AUTO_TEST_CASE(switch_drop_sound)
{
	BOOST_TEST(compile_from_files("common/switch_drop_sound"));
}

BOOST_AUTO_TEST_CASE(simple_price_queries)
{
	using lang::market::divination_card;
	using lang::market::price_data;

	lang::market::item_price_data ipd;
	ipd.divination_cards.push_back(divination_card{price_data{0.125, false}, "Rain of Chaos", 8});
	ipd.divination_cards.push_back(divination_card{price_data{5, false}, "Humility", 9});
	ipd.divination_cards.push_back(divination_card{price_data{10, false}, "A Dab of Ink", 9});
	ipd.divination_cards.push_back(divination_card{price_data{100, false}, "Abandoned Wealth", 5});
	ipd.divination_cards.push_back(divination_card{price_data{1000, false}, "The Doctor", 8});
	BOOST_TEST(compile_from_files("common/simple_price_queries", {}, ipd));
}

BOOST_AUTO_TEST_CASE(override_settings_font_min)
{
	compiler::settings st;
	st.overrides.font.show_size_min = 26;
	st.overrides.font.hide_size_min = 22;

	BOOST_TEST(compile_from_files(
		"common/override_settings_font.filtertemplate",
		"common/override_settings_font_min.filter",
		st));
}

BOOST_AUTO_TEST_CASE(override_settings_font_max)
{
	compiler::settings st;
	st.overrides.font.show_size_max = 26;
	st.overrides.font.hide_size_max = 22;

	BOOST_TEST(compile_from_files(
		"common/override_settings_font.filtertemplate",
		"common/override_settings_font_max.filter",
		st));
}

BOOST_AUTO_TEST_CASE(override_settings_font_min_max)
{
	compiler::settings st;
	st.overrides.font.show_size_min = 24;
	st.overrides.font.show_size_max = 26;
	st.overrides.font.hide_size_min = 18;
	st.overrides.font.hide_size_max = 22;

	BOOST_TEST(compile_from_files(
		"common/override_settings_font.filtertemplate",
		"common/override_settings_font_min_max.filter",
		st));
}

BOOST_AUTO_TEST_CASE(override_settings_opacity_min_max)
{
	compiler::settings st;
	st.overrides.color.show_opacity_min = 245;
	st.overrides.color.show_opacity_max = 250;
	st.overrides.color.hide_opacity_min = 160;
	st.overrides.color.hide_opacity_max = 200;

	BOOST_TEST(compile_from_files("common/override_settings_opacity_min_max", st));
}

BOOST_AUTO_TEST_CASE(override_settings_opacity_min_max_all_actions)
{
	compiler::settings st;
	st.overrides.color.show_opacity_min = 245;
	st.overrides.color.show_opacity_max = 250;
	st.overrides.color.hide_opacity_min = 160;
	st.overrides.color.hide_opacity_max = 200;
	st.overrides.color.override_all_actions = true;

	BOOST_TEST(compile_from_files(
		"common/override_settings_opacity_min_max.filtertemplate",
		"common/override_settings_opacity_min_max_all_actions.filter",
		st));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
