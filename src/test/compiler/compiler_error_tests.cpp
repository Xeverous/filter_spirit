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

namespace fs::test
{

using compiler::diagnostic_message;
using compiler::diagnostics_store;
using dmid = compiler::diagnostic_message_id;
using dms = compiler::diagnostic_message_severity;

BOOST_FIXTURE_TEST_SUITE(compiler_suite, compiler_fixture)

	BOOST_AUTO_TEST_CASE(minimal_input_resolve_constants)
	{
		const parser::parsed_spirit_filter parse_data = parse(minimal_input());
		diagnostics_store diagnostics;
		const std::optional<compiler::symbol_table> symbols =
			resolve_symbols(parse_data.ast.definitions, diagnostics);
		BOOST_TEST_REQUIRE(!diagnostics.has_errors());
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
		static diagnostics_store expect_error_when_resolving_symbols(
			const std::vector<parser::ast::sf::definition>& defs)
		{
			diagnostics_store diagnostics;
			resolve_symbols(defs, diagnostics);
			BOOST_TEST_REQUIRE(diagnostics.has_errors());
			return diagnostics;
		}

		static diagnostics_store expect_error_when_compiling(
			const parser::ast::sf::filter_structure& fs)
		{
			compiler::settings st;
			diagnostics_store diagnostics;
			const std::optional<compiler::symbol_table> symbols = resolve_symbols(st, fs.definitions, diagnostics);
			BOOST_TEST(!diagnostics.has_warnings_or_errors());
			BOOST_TEST_REQUIRE(symbols.has_value());
			(void) compiler::compile_spirit_filter_statements(st, fs.statements, *symbols, diagnostics);
			BOOST_TEST(diagnostics.has_warnings_or_errors());
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
		const diagnostics_store& diagnostics,
		const parser::parse_metadata& metadata)
	{
		auto dump_diagnostics = [&]() {
			log::string_logger logger;
			diagnostics.output_messages(metadata, logger);
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
				actual.origin.map([&](lang::position_tag origin) { return metadata.lookup.text_of(origin); });

			if (expected.underline != actual_underline) {
				if (expected.underline.has_value() && actual_underline.has_value()) {
					// if both underlines have value, use a dedicated comparison code
					BOOST_TEST(compare_ranges(*expected.underline, *actual_underline, metadata.lookup.whole_text()));
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
			const diagnostics_store diagnostics = expect_error_when_resolving_symbols(parse_data.ast.definitions);

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
			const diagnostics_store diagnostics = expect_error_when_resolving_symbols(parse_data.ast.definitions);

			const std::string_view expected_name = search(input, "$non_existent_obj").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::no_such_name, expected_name, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(unknown_expression)
		{
			const std::string input_str = minimal_input() + R"(
$val = NoSuchKeyword
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_resolving_symbols(parse_data.ast.definitions);

			const std::string_view expected_error = search(input, "NoSuchKeyword").result();

			std::vector<diagnostic_message_pattern> patterns = {
				// currently socket_spec alternative takes priority over any possible unknown expression
				diagnostic_message_pattern{dms::error, dmid::invalid_socket_spec, expected_error, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_amount_of_arguments)
		{
			const std::string input_str = minimal_input() + R"(PlayAlertSound 11 22 33)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

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
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_argument = search(input, "3").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::value_out_of_range, expected_argument, {
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
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

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
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_first = search(input, "Shaper").result();
			const std::string_view expected_second = search(input, "Shaper").next().result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::warning, dmid::duplicate_influence, expected_second, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_first, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_statement_action_after_condition)
		{
			const std::string input_str = minimal_input() + R"(
Class == "Skill Gems" {
	SetTextColor 0 255 0
	Show
}

Class == "Support Gems"
SetTextColor 0 240 0
Show
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_expression = search(input, "SetTextColor 0 240 0").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::invalid_statement, expected_expression, {"action"}},
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_statement_expansion_after_condition)
		{
			const std::string input_str = minimal_input() + R"(
$x = {
	SetTextColor 0 255 0
}

Class == "Skill Gems" {
	Expand $x
	Show
}

Class == "Support Gems"
Expand $x
Show
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_expression = search(input, "Expand $x").next().result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::invalid_statement, expected_expression, {"expansion"}},
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_statement_visibility_after_condition)
		{
			const std::string input_str = minimal_input() + R"(
Class == "Skill Gems" {
	Show
}

Class == "Support Gems"
Hide
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_expression = search(input, "Hide").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::invalid_statement, expected_expression, {"visibility"}},
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(unknown_statement)
		{
			const std::string input_str = minimal_input() + R"(
Class == "Skill Gems" {
	UnknownStatement True
	Show
}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_expression = search(input, "UnknownStatement").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::unknown_statement, expected_expression, {}},
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(condition_redefinition)
		{
			const std::string input_str = minimal_input() + R"(
Corrupted False
Mirrored True
Corrupted True
{}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_original = search(input, "Corrupted False").result();
			const std::string_view expected_redef = search(input, "Corrupted True").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::condition_redefinition, expected_redef, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_original, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(lower_bound_redefinition)
		{
			const std::string input_str = minimal_input() + R"(
Quality > 10
Quality >= 5
{}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_original = search(input, "Quality > 10").result();
			const std::string_view expected_redef = search(input, "Quality >= 5").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::lower_bound_redefinition, expected_redef, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_original, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(upper_bound_redefinition)
		{
			const std::string input_str = minimal_input() + R"(
Quality < 10
Quality <= 5
{}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_original = search(input, "Quality < 10").result();
			const std::string_view expected_redef = search(input, "Quality <= 5").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::upper_bound_redefinition, expected_redef, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_original, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(condition_after_equality)
		{
			const std::string input_str = minimal_input() + R"(
Quality = 10
Quality > 0
{}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_original = search(input, "Quality = 10").result();
			const std::string_view expected_redef = search(input, "Quality > 0").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::condition_after_equality, expected_redef, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_original, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(dead_condition)
		{
			const std::string input_str = minimal_input() + R"(
Prophecy ""
GemQualityType ""
AlternateQuality True
ArchnemesisMod "Toxic"
{}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_prop = search(input, "Prophecy").result();
			const std::string_view expected_gem = search(input, "GemQualityType").result();
			const std::string_view expected_quality = search(input, "AlternateQuality True").result();
			const std::string_view expected_arch = search(input, "ArchnemesisMod \"Toxic\"").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::dead_condition, expected_prop, {}},
				diagnostic_message_pattern{dms::error, dmid::dead_condition, expected_gem, {}},
				diagnostic_message_pattern{dms::warning, dmid::dead_condition, expected_quality, {}},
				diagnostic_message_pattern{dms::warning, dmid::dead_condition, expected_arch, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(unknown_invalid_expression)
		{
			const std::string input_str = minimal_input() + R"(
Class >=3 "Helmet"
Autogen "Stuff"
ItemLevel > 0xFF
{}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_error_class = search(input, "3").result();
			const std::string_view expected_error_autogen = search(input, "\"Stuff\"").result();
			const std::string_view expected_error_ilvl = search(input, "xFF").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::invalid_expression, expected_error_class, {}},
				diagnostic_message_pattern{dms::error, dmid::invalid_expression, expected_error_autogen, {}},
				// currently socket_spec alternative takes priority over any possible unknown expression
				diagnostic_message_pattern{dms::error, dmid::invalid_socket_spec, expected_error_ilvl, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_operator)
		{
			const std::string input_str = minimal_input() + R"(
Identified >= False
HasInfluence != Shaper
Class <= "Belt"
HasExplicitMod != "Veiled" # != is not supported
HasExplicitMod >= "Veiled" # only equality may skip count
Price != 10
{}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_error_id = search(input, ">=").result();
			const std::string_view expected_error_infl = search(input, "!=").result();
			const std::string_view expected_error_class = search(input, "<=").result();
			const std::string_view expected_error_mod1 = search(input, "!=").next().result();
			const std::string_view expected_error_mod2 = search(input, ">=").next().result();
			const std::string_view expected_error_price = search(input, "!=").next().result();

			std::vector<diagnostic_message_pattern> patterns = {
				// price is first because it is analyzed immediately (not stored for later processing)
				diagnostic_message_pattern{dms::error, dmid::invalid_operator, expected_error_id, {}},
				diagnostic_message_pattern{dms::error, dmid::invalid_operator, expected_error_infl, {}},
				diagnostic_message_pattern{dms::error, dmid::invalid_operator, expected_error_class, {}},
				diagnostic_message_pattern{dms::error, dmid::invalid_operator, expected_error_mod1, {}},
				diagnostic_message_pattern{dms::error, dmid::invalid_operator, expected_error_mod2, {}},
				diagnostic_message_pattern{dms::error, dmid::invalid_operator, expected_error_price, {}},
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
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

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
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

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
				diagnostic_message_pattern{dms::error, dmid::value_out_of_range, expected_origin, {
					"1 - 16", "got 17"
				}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(invalid_play_effect)
		{
			const std::string input_str = minimal_input() + R"(
PlayEffect Temp
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_origin = search(input, "Temp").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::invalid_play_effect, expected_origin, {}},
				diagnostic_message_pattern{dms::note, dmid::attempt_description, boost::none, {}},
				diagnostic_message_pattern{dms::error, dmid::type_mismatch, expected_origin, {}},
				diagnostic_message_pattern{dms::note, dmid::attempt_description, boost::none, {}},
				diagnostic_message_pattern{dms::error, dmid::type_mismatch, expected_origin, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

		BOOST_AUTO_TEST_CASE(autogen)
		{
			const std::string input_str = minimal_input() + R"(
Price > 10 {
	Show
}

Autogen "oils" {
	Hide
}
)";
			const std::string_view input = input_str;
			const parser::parsed_spirit_filter parse_data = parse(input);
			const diagnostics_store diagnostics = expect_error_when_compiling(parse_data.ast);

			const std::string_view expected_price_error = search(input, "Show").result();
			const std::string_view expected_price_origin = search(input, "Price > 10").result();
			const std::string_view expected_autogen_error = search(input, "Hide").result();
			const std::string_view expected_autogen_origin = search(input, "Autogen \"oils\"").result();

			std::vector<diagnostic_message_pattern> patterns = {
				diagnostic_message_pattern{dms::error, dmid::price_without_autogen, expected_price_error, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_price_origin, {}},
				diagnostic_message_pattern{dms::warning, dmid::autogen_without_price, expected_autogen_error, {}},
				diagnostic_message_pattern{dms::note, dmid::minor_note, expected_autogen_origin, {}}
			};

			compare_diagnostics(patterns, diagnostics, parse_data.metadata);
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
