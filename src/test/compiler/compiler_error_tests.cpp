#include "common/print_type.hpp"
#include "common/test_fixtures.hpp"
#include "common/string_operations.hpp"

#include <fs/compiler/compiler.hpp>
#include <fs/compiler/diagnostics.hpp>
#include <fs/log/string_logger.hpp>
#include <fs/utility/string_helpers.hpp>
#include <fs/utility/type_traits.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/optional/optional_io.hpp>

#include <string>
#include <string_view>
#include <functional>
#include <utility>
#include <stdexcept>
#include <exception>

namespace ut = boost::unit_test;
namespace tt = boost::test_tools;

namespace fs::test
{

using compiler::diagnostic_message;
using compiler::diagnostics_container;
using dmid = compiler::diagnostic_message_id;
using dms = compiler::diagnostic_message_severity;

BOOST_FIXTURE_TEST_SUITE(compiler_suite, compiler_fixture)

	BOOST_AUTO_TEST_CASE(minimal_input_resolve_constants)
	{
		const parser::parsed_spirit_filter parse_data = parse(minimal_input());
		diagnostics_container diagnostics;
		const boost::optional<compiler::symbol_table> symbols =
			resolve_symbols(parse_data.ast.definitions, diagnostics);
		BOOST_TEST_REQUIRE(!compiler::has_errors(diagnostics));
		BOOST_TEST_REQUIRE(symbols.has_value());
		BOOST_TEST((*symbols).objects.empty());
		BOOST_TEST((*symbols).trees.empty());
	}

	/*
	 * tests that check for correct compiler errors for common mistakes
	 *
	 * - parse should succeed
	 * - compilation step(s) should fail
	 *   - check the the error variant holds correct error type
	 *   - check that error information points to relevant place in input
	 */
	class compiler_error_fixture : public compiler_fixture
	{
	protected:
		static diagnostics_container expect_error_when_resolving_symbols(
			const std::vector<parser::ast::sf::definition>& defs)
		{
			diagnostics_container diagnostics;
			resolve_symbols(defs, diagnostics);
			BOOST_TEST_REQUIRE(compiler::has_errors(diagnostics));
			return diagnostics;
		}

		static diagnostics_container expect_error_when_compiling(
			const parser::ast::sf::filter_structure& fs)
		{
			compiler::settings st;
			diagnostics_container diagnostics;
			const boost::optional<compiler::symbol_table> symbols = resolve_symbols(st, fs.definitions, diagnostics);
			BOOST_TEST(!compiler::has_warnings_or_errors(diagnostics));
			BOOST_TEST_REQUIRE(symbols.has_value());
			(void) compiler::compile_spirit_filter_statements(st, fs.statements, *symbols, diagnostics);
			BOOST_TEST(compiler::has_errors(diagnostics));
			return diagnostics;
		}
	};

	struct diagnostic_message_pattern
	{
		dms severity;
		dmid id;
		boost::optional<std::string_view> underline;
		std::vector<std::string> description_substrings;
	};

	void compare_diagnostics(
		std::vector<diagnostic_message_pattern> patterns,
		const diagnostics_container& diagnostics,
		const parser::parse_metadata& metadata)
	{
		auto dump_diagnostics = [&]() {
			log::string_logger logger;
			compiler::output_diagnostics(diagnostics, metadata, logger);
			return logger.str();
		};

		// there might be more errors caused by the previous ones, hence >= instead of ==
		BOOST_TEST(diagnostics.size() >= patterns.size()); // report comparison using boost test built-in logic
		BOOST_TEST_REQUIRE(diagnostics.size() >= patterns.size(), "DIAGNOSTICS:\n" << dump_diagnostics()); // report all diagnostics

		for (std::size_t i = 0; i < patterns.size(); ++i) {
			const diagnostic_message_pattern& expected = patterns[i];
			const diagnostic_message& actual = diagnostics[i];

			BOOST_TEST(expected.severity == actual.severity);
			BOOST_TEST(expected.id == actual.id);

			const boost::optional<std::string_view> actual_underline =
				actual.origin.map([&](lang::position_tag origin) { return metadata.lookup.position_of(origin); });

			if (expected.underline != actual_underline) {
				if (expected.underline.has_value() && actual_underline.has_value()) {
					// if both underlines have value, use a dedicated comparison code
					BOOST_TEST(compare_ranges(*expected.underline, *actual_underline, metadata.lookup.get_view_of_whole_content()));
				}
				else {
					BOOST_ERROR("underlines differ, "
						"EXPECTED:\n" << expected.underline.value_or("(none)") << "\n"
						"ACTUAL:\n" << actual_underline.value_or("(none)"));
				}
			}

			for (const auto& str : expected.description_substrings) {
				if (!utility::contains(actual.description, str)) {
					BOOST_ERROR("underline \"" << actual.description << "\" does not contain \"" << str << "\"");
				}
			}
		}
	}

	BOOST_FIXTURE_TEST_SUITE(compiler_error_suite, compiler_error_fixture,
		* ut::depends_on("compiler_suite/minimal_input_resolve_constants"))

		BOOST_AUTO_TEST_CASE(name_already_exists)
		{
			const std::string input_str = minimal_input() + R"(
$some_var = 0
$xyz = 1
$some_other_var = 2
$xyz = 3
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_resolving_symbols(parse_data.ast.definitions);

			auto xyz_search = search(input, "$xyz");
			const std::string_view expected_original_name = xyz_search.result();
			const std::string_view expected_duplicated_name = xyz_search.next().result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::name_already_exists, expected_original_name, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_duplicated_name, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(no_such_name)
		{
			const std::string input_str = minimal_input() + R"(
$abc = 0
$xyz = $non_existent_obj
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_resolving_symbols(parse_data.ast.definitions);

			const std::string_view expected_name = search(input, "$non_existent_obj").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::no_such_name, expected_name, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_amount_of_arguments)
		{
			const std::string input_str = minimal_input() + R"(PlayAlertSound 11 22 33)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_arguments = search(input, "11 22 33").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::invalid_amount_of_arguments, expected_arguments, {
					"from 1 to 2", "got 3"
				}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_integer_value)
		{
			const std::string input_str = minimal_input() + R"(MinimapIcon 3 Brown Circle)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_argument = search(input, "3").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::invalid_integer_value, expected_argument, {
					"0 - 2", "got 3"
				}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(type_mismatch)
		{
			const std::string input_str = minimal_input() + R"(BaseType 123 {})";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_expression = search(input, "123").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::type_mismatch, expected_expression, {
					"'string'", "got 'integer'"
				}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(duplicate_influence)
		{
			const std::string input_str = minimal_input() + R"(HasInfluence Shaper Shaper {})";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_first = search(input, "Shaper").result();
			const std::string_view expected_second = search(input, "Shaper").next().result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::duplicate_influence, expected_second, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_first, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(condition_redefinition)
		{
			const std::string input_str = minimal_input() + R"(
Class "Boots"
BaseType "Dragonscale"
Class "Gloves"
{}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_original = search(input, "Class \"Boots\"").result();
			const std::string_view expected_redef = search(input, "Class \"Gloves\"").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::condition_redefinition, expected_redef, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_original, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(lower_bound_redefinition)
		{
			const std::string input_str = minimal_input() + R"(
Quality = 10
Quality > 0
{}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_original = search(input, "Quality = 10").result();
			const std::string_view expected_redef = search(input, "Quality > 0").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::lower_bound_redefinition, expected_redef, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_original, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_alert_sound_id)
		{
			const std::string input_str = minimal_input() + R"(
PlayAlertSound True
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_origin = search(input, "True").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::invalid_alert_sound_id, expected_origin, {}},
				diagnostic_message_pattern{dms::note, dmid::attempt_description, boost::none, {"None"}},
				diagnostic_message_pattern{dms::error, dmid::type_mismatch, expected_origin, {}},
				diagnostic_message_pattern{dms::note, dmid::attempt_description, boost::none, {"integer"}},
				diagnostic_message_pattern{dms::error, dmid::type_mismatch, expected_origin, {}},
				diagnostic_message_pattern{dms::note, dmid::attempt_description, boost::none, {"Shaper"}},
				diagnostic_message_pattern{dms::error, dmid::type_mismatch, expected_origin, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_set_alert_sound)
		{
			const std::string input_str = minimal_input() + R"(
SetAlertSound 17
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_container diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_origin = search(input, "17").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::invalid_set_alert_sound, expected_origin, {}},
				diagnostic_message_pattern{dms::note, dmid::attempt_description, boost::none, {
					lang::keywords::rf::custom_alert_sound
				}},
				diagnostic_message_pattern{dms::error, dmid::type_mismatch, expected_origin, {}},
				diagnostic_message_pattern{dms::note, dmid::attempt_description, boost::none, {
					lang::keywords::rf::play_alert_sound
				}},
				diagnostic_message_pattern{dms::error, dmid::invalid_integer_value, expected_origin, {
					"1 - 16", "got 17"
				}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
