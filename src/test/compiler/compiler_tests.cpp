#include "common/print_type.hpp"
#include "common/test_fixtures.hpp"
#include "common/string_operations.hpp"

#include <fs/compiler/compiler.hpp>
#include <fs/compiler/outcome.hpp>
#include <fs/generator/make_filter.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/log/string_logger.hpp>
#include <fs/utility/visitor.hpp>

#include <boost/test/unit_test.hpp>

#include <initializer_list>
#include <string_view>
#include <utility>

namespace ut = boost::unit_test;

namespace fs::test
{

// note: object's internal origin is not checked
// use the second pair element
void expect_object(
	const lang::symbol_table& symbols,
	const parser::lookup_data& lookup_data,
	const std::string& name,
	std::string_view expected_name_origin,
	std::initializer_list<std::pair<lang::object_variant, std::string_view>> expected_obj)
{
	const auto it = symbols.find(name);
	if (it == symbols.end()) {
		BOOST_ERROR("expected " << name << " in symbol table");
		return;
	}

	const lang::named_object& named_object = it->second;
	const lang::object& actual_obj = named_object.object_instance;
	const std::string_view all_code = lookup_data.get_view_of_whole_content();

	const std::string_view actual_name_origin = lookup_data.position_of(named_object.name_origin);
	BOOST_TEST(compare_ranges(expected_name_origin, actual_name_origin, all_code));

	BOOST_TEST_REQUIRE(
		actual_obj.values.size() == expected_obj.size(),
		"objects should have the same number of values");

	for (std::size_t i = 0; i < actual_obj.values.size(); ++i) {
		const auto expected_origin = expected_obj.begin()[i].second;
		const auto actual_origin = lookup_data.position_of(actual_obj.values[i].origin);
		BOOST_TEST(compare_ranges(expected_origin, actual_origin, all_code));

		if (expected_obj.begin()[i].first != actual_obj.values[i].value) {
			BOOST_ERROR(
				"expected " << lang::object_type_of(expected_obj.begin()[i].first) <<
				" but got " << actual_obj.values[i].type() << "\n"
				<< utility::range_underline_to_string(all_code, expected_origin)
				<< utility::range_underline_to_string(all_code, actual_origin));
		}
	}
}

BOOST_FIXTURE_TEST_SUITE(compiler_suite, compiler_fixture)

	class compiler_success_fixture : public compiler_fixture
	{
	protected:
		static
		lang::symbol_table expect_success_when_resolving_symbols(
			const std::vector<parser::ast::sf::definition>& defs,
			const parser::lookup_data& lookup_data)
		{
			compiler::outcome<lang::symbol_table> symbols_outcome = resolve_symbols(defs);

			if (compiler::has_errors(symbols_outcome.logs())) {
				log::string_logger logger;
				compiler::output_logs(symbols_outcome.logs(), lookup_data, logger);
				BOOST_FAIL("resolve_symbols failed but should not:\n" << logger.str());
			}

			BOOST_TEST_REQUIRE(symbols_outcome.has_result());
			return std::move(symbols_outcome).result();
		}

		static
		lang::item_filter expect_success_when_building_filter(
			const std::vector<parser::ast::sf::statement>& top_level_statements,
			const parser::lookup_data& lookup_data,
			const lang::symbol_table& symbols)
		{
			compiler::outcome<lang::spirit_item_filter> sf_outcome =
				compiler::compile_spirit_filter_statements(compiler::settings{}, top_level_statements, symbols);

			if (compiler::has_errors(sf_outcome.logs())) {
				log::string_logger logger;
				compiler::output_logs(sf_outcome.logs(), lookup_data, logger);
				BOOST_FAIL("building spirit filter blocks failed but should not:\n" << logger.str());
			}

			BOOST_TEST_REQUIRE(sf_outcome.has_result());
			return generator::make_filter(sf_outcome.result(), /* empty item price data */ {});
		}
	};

	BOOST_FIXTURE_TEST_SUITE(compiler_success_suite, compiler_success_fixture,
		* ut::depends_on("compiler_suite/minimal_input_resolve_constants"))

		BOOST_AUTO_TEST_CASE(all_possible_constants,
			* ut::description("test that all types can be used as constant"))
		{
			const std::string input_str = minimal_input() + R"(
$none           = None
$temp           = Temp
$boolean        = False
$fractional     = 3.5
$integer        = 123
$socket_spec    = 5RGB
$influence      = Elder
$rarity         = Rare
$shape          = Hexagon
$suit           = Yellow
$shaper_voice   = ShChaos
$string         = "Leather Belt"
)";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const parser::lookup_data& lookup_data = parse_data.lookup;
			const lang::symbol_table symbols = expect_success_when_resolving_symbols(parse_data.ast.definitions, lookup_data);

			expect_object(symbols, lookup_data, "none", search(input, "$none").result(),
				{{lang::none{}, search(input, "None").result()}}
			);
			expect_object(symbols, lookup_data, "temp", search(input, "$temp").result(),
				{{lang::temp{}, search(input, "Temp").result()}}
			);
			expect_object(symbols, lookup_data, "boolean", search(input, "$boolean").result(),
				{{lang::boolean{false}, search(input, "False").result()}}
			);
			expect_object(symbols, lookup_data, "fractional", search(input, "$fractional").result(),
				{{lang::fractional{3.5}, search(input, "3.5").result()}}
			);
			expect_object(symbols, lookup_data, "integer", search(input, "$integer").result(),
				{{lang::integer{123}, search(input, "123").result()}}
			);
			expect_object(symbols, lookup_data, "socket_spec", search(input, "$socket_spec").result(),
				{{lang::socket_spec{5, 1, 1, 1}, search(input, "5RGB").result()}}
			);
			expect_object(symbols, lookup_data, "influence", search(input, "$influence").result(),
				{{lang::influence{lang::influence_type::elder}, search(input, "Elder").result()}}
			);
			expect_object(symbols, lookup_data, "rarity", search(input, "$rarity").result(),
				{{lang::rarity{lang::rarity_type::rare}, search(input, "Rare").result()}}
			);
			expect_object(symbols, lookup_data, "shape", search(input, "$shape").result(),
				{{lang::shape{lang::shape_type::hexagon}, search(input, "Hexagon").result()}}
			);
			expect_object(symbols, lookup_data, "suit", search(input, "$suit").result(),
				{{lang::suit{lang::suit_type::yellow}, search(input, "Yellow").result()}}
			);
			expect_object(symbols, lookup_data, "shaper_voice", search(input, "$shaper_voice").result(),
				{{lang::shaper_voice_line{lang::shaper_voice_line_type::chaos}, search(input, "ShChaos").result()}}
			);
			expect_object(symbols, lookup_data, "string", search(input, "$string").result(),
				{{lang::string{"Leather Belt"}, search(input, "\"Leather Belt\"").result()}}
			);
		}

		BOOST_AUTO_TEST_CASE(sequence_integer_definitions,
			* ut::description("test that sequences of various lengths are correctly interpreted"))
		{
			const std::string input_str = minimal_input() + R"(
$x = 1 2
$y = 3 4 5
$z = 6 7 8 9
)";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const parser::lookup_data& lookup_data = parse_data.lookup;
			const lang::symbol_table symbols = expect_success_when_resolving_symbols(parse_data.ast.definitions, lookup_data);

			expect_object(
				symbols, lookup_data, "x", search(input, "$x").result(), {
					{lang::integer{1}, search(input, "1").result()},
					{lang::integer{2}, search(input, "2").result()}
				}
			);
			expect_object(
				symbols, lookup_data, "y", search(input, "$y").result(), {
					{lang::integer{3}, search(input, "3").result()},
					{lang::integer{4}, search(input, "4").result()},
					{lang::integer{5}, search(input, "5").result()},
				}
			);
			expect_object(
				symbols, lookup_data, "z", search(input, "$z").result(), {
					{lang::integer{6}, search(input, "6").result()},
					{lang::integer{7}, search(input, "7").result()},
					{lang::integer{8}, search(input, "8").result()},
					{lang::integer{9}, search(input, "9").result()}
				}
			);
		}

		BOOST_AUTO_TEST_CASE(sequence_socket_spec_definitions)
		{
			const std::string input_str = minimal_input() + R"(
$x = 1 RR
$y = 3GGG 2
$z = 4W 0 6 RGB
)";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const parser::lookup_data& lookup_data = parse_data.lookup;
			const lang::symbol_table symbols = expect_success_when_resolving_symbols(parse_data.ast.definitions, lookup_data);

			expect_object(
				symbols, lookup_data, "x", search(input, "$x").result(), {
					{lang::integer{1}, search(input, "1").result()},
					{lang::socket_spec{std::nullopt, 2}, search(input, "RR").result()}
				}
			);
			expect_object(
				symbols, lookup_data, "y", search(input, "$y").result(), {
					{lang::socket_spec{3, 0, 3}, search(input, "3GGG").result()},
					{lang::integer{2}, search(input, "2").result()},
				}
			);
			expect_object(
				symbols, lookup_data, "z", search(input, "$z").result(), {
					{lang::socket_spec{4, 0, 0, 0, 1}, search(input, "4W").result()},
					{lang::integer{0}, search(input, "0").result()},
					{lang::integer{6}, search(input, "6").result()},
					{lang::socket_spec{std::nullopt, 1, 1, 1}, search(input, "RGB").result()}
				}
			);
		}

		class alert_sound_fixture : public compiler_success_fixture
		{
		private:
			static
			void test_alert_sound(
				const lang::alert_sound& expected_alert_sound,
				const lang::alert_sound& actual_alert_sound)
			{
				std::visit(utility::visitor{
					[&](const lang::builtin_alert_sound& expected)
					{
						if (!std::holds_alternative<lang::builtin_alert_sound>(actual_alert_sound.sound))
							BOOST_ERROR("expected built-in alert sound but got different");

						const auto& actual = std::get<lang::builtin_alert_sound>(actual_alert_sound.sound);
						BOOST_TEST((actual.sound_id.id == expected.sound_id.id));
						BOOST_TEST((actual.volume == expected.volume));
						BOOST_TEST(actual.is_positional == expected.is_positional);
					},
					[&](const lang::custom_alert_sound& expected)
					{
						if (!std::holds_alternative<lang::custom_alert_sound>(actual_alert_sound.sound))
							BOOST_ERROR("expected custom alert sound but got different");

						const auto& actual = std::get<lang::custom_alert_sound>(actual_alert_sound.sound);
						BOOST_TEST(actual.path.value == expected.path.value);
					}
				}, expected_alert_sound.sound);
			}

			static
			void expect_alert_sound_impl(
				std::string action,
				const lang::alert_sound& expected_value)
			{
				const std::string input_str = minimal_input() + action + "\nShow";
				const std::string_view input = input_str;
				const parser::sf::parse_success_data parse_data = parse(input);
				const parser::lookup_data& lookup_data = parse_data.lookup;
				const lang::symbol_table symbols =
					expect_success_when_resolving_symbols(parse_data.ast.definitions, lookup_data);
				const lang::item_filter filter =
					expect_success_when_building_filter(parse_data.ast.statements, parse_data.lookup, symbols);
				BOOST_TEST_REQUIRE(static_cast<int>(filter.blocks.size()) == 1);
				const lang::item_filter_block& block = filter.blocks[0];
				BOOST_TEST(block.visibility.show == true);
				const std::optional<lang::alert_sound_action>& maybe_alert_sound_action = block.actions.play_alert_sound;
				BOOST_TEST_REQUIRE(maybe_alert_sound_action.has_value());
				const lang::alert_sound& alert_sound = (*maybe_alert_sound_action).alert;
				test_alert_sound(expected_value, alert_sound);
			}

		public:
			static
			void expect_builtin_alert_sound(
				std::string expression,
				lang::builtin_alert_sound_id sound_id,
				std::optional<lang::integer> volume = std::nullopt)
			{
				expect_alert_sound_impl(
					"PlayAlertSound " + expression,
					lang::alert_sound{lang::builtin_alert_sound{false, sound_id, volume}});
				expect_alert_sound_impl(
					"PlayAlertSoundPositional " + expression,
					lang::alert_sound{lang::builtin_alert_sound{true, sound_id, volume}});
			}

			static
			void expect_custom_alert_sound(
				std::string expression,
				const lang::string& path)
			{
				expect_alert_sound_impl(
					"CustomAlertSound " + expression,
					lang::alert_sound{lang::custom_alert_sound{path}});
			}
		};

		BOOST_FIXTURE_TEST_SUITE(alert_sound_suite, alert_sound_fixture)

			using namespace lang;

			BOOST_AUTO_TEST_CASE(builtin_sound_id_integer)
			{
				expect_builtin_alert_sound("1", lang::builtin_alert_sound_id{integer{1}});
			}

			BOOST_AUTO_TEST_CASE(builtin_sound_id_and_volume_integer)
			{
				expect_builtin_alert_sound("1 300", lang::builtin_alert_sound_id{integer{1}}, integer{300});
			}

			BOOST_AUTO_TEST_CASE(builtin_sound_id_shaper)
			{
				expect_builtin_alert_sound("ShGeneral", lang::builtin_alert_sound_id{shaper_voice_line{shaper_voice_line_type::general}});
			}

			BOOST_AUTO_TEST_CASE(builtin_sound_id_and_volume_shaper)
			{
				expect_builtin_alert_sound("ShGeneral 300", lang::builtin_alert_sound_id{shaper_voice_line{shaper_voice_line_type::general}}, integer{300});
			}

			BOOST_AUTO_TEST_CASE(custom_path)
			{
				expect_custom_alert_sound("\"pop.wav\"", string{"pop.wav"});
			}

		BOOST_AUTO_TEST_SUITE_END()

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
