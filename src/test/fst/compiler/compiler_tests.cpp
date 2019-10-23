#include <fst/common/print_type.hpp>
#include <fst/common/test_fixtures.hpp>
#include <fst/common/string_operations.hpp>

#include <fs/compiler/build_filter_blocks.hpp>
#include <fs/compiler/resolve_symbols.hpp>
#include <fs/compiler/print_error.hpp>
#include <fs/lang/position_tag.hpp>
#include <fs/log/buffered_logger.hpp>
#include <fs/utility/visitor.hpp>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string_view>
#include <utility>

namespace ut = boost::unit_test;
using namespace fs;

namespace fst
{

void expect_object_in_symbols(
	const lang::symbol_table& symbols,
	const parser::lookup_data& lookup_data,
	const std::string& name,
	const lang::object_variant& value,
	std::string_view name_origin,
	std::string_view value_origin)
{
	const auto it = symbols.find(name);
	if (it == symbols.end())
	{
		BOOST_ERROR("expected " << name << " in symbol table");
		return;
	}

	const lang::named_object& named_object = it->second;
	const lang::object& object = named_object.object_instance;
	if (object.value != value)
	{
		const std::string_view all_code = lookup_data.get_view_of_whole_content();
		BOOST_ERROR("expected " << lang::type_of_object(value) << " but got " << lang::type_of_object(object.value) << "\n"
			<< range_info_to_string(all_code, lookup_data.position_of(named_object.name_origin))
			<< range_info_to_string(all_code, lookup_data.position_of(object.value_origin)));
	}

	const std::string_view input_range = lookup_data.get_view_of_whole_content();
	const std::string_view object_name_origin = lookup_data.position_of(named_object.name_origin);
	const std::string_view object_value_origin = lookup_data.position_of(object.value_origin);
	BOOST_TEST(compare_ranges(name_origin, object_name_origin, input_range));
	BOOST_TEST(compare_ranges(value_origin, object_value_origin, input_range));
}

BOOST_FIXTURE_TEST_SUITE(compiler_suite, compiler_fixture)

	class compiler_success_fixture : public compiler_fixture
	{
	protected:
		static
		lang::symbol_table expect_success_when_resolving_symbols(
			const std::vector<parser::ast::definition>& defs,
			const parser::lookup_data& lookup_data)
		{
			const std::variant<lang::symbol_table, compiler::compile_error> symbols_or_error =
				resolve_symbols(defs);

			if (std::holds_alternative<compiler::compile_error>(symbols_or_error))
			{
				const auto& error = std::get<compiler::compile_error>(symbols_or_error);
				log::buffered_logger logger;
				compiler::print_error(error, lookup_data, logger);
				const auto log = logger.flush_out();
				BOOST_FAIL("resolve_symbols failed but should not:\n" << log);
			}

			return std::get<lang::symbol_table>(std::move(symbols_or_error));
		}

		static
		std::vector<lang::filter_block> expect_success_when_building_filter(
			const std::vector<parser::ast::statement>& top_level_statements,
			const parser::lookup_data& lookup_data,
			const lang::symbol_table& symbols)
		{
			std::variant<std::vector<lang::filter_block>, compiler::compile_error> result =
				compiler::build_filter_blocks(top_level_statements, symbols, lang::item_price_data{});

			if (std::holds_alternative<compiler::compile_error>(result))
			{
				const auto& error = std::get<compiler::compile_error>(result);
				log::buffered_logger logger;
				compiler::print_error(error, lookup_data, logger);
				const auto log = logger.flush_out();
				BOOST_FAIL("building filter blocks failed but should not:\n" << log);
			}

			return std::get<std::vector<lang::filter_block>>(std::move(result));
		}
	};

	BOOST_FIXTURE_TEST_SUITE(compiler_success_suite, compiler_success_fixture,
		* ut::depends_on("compiler_suite/minimal_input_resolve_constants"))

		BOOST_AUTO_TEST_CASE(all_possible_constants,
			* ut::description("test that all types can be used as constant"))
		{
			const std::string input_str = minimal_input() + R"(
nothing        = _
boolean        = false
floating_point = 3.5
integer        = 123
level          = Level(10)
font_size      = FontSize(30)
sound_id       = SoundId(7)
volume         = Volume(300)
group          = Group("RGB")
rarity         = rare
shape          = hexagon
suit           = brown
color          = RGB(101, 102, 103)
icon           = MinimapIcon(0, green, square)
beam           = Beam(yellow)
string         = "Leather Belt"
path           = Path("pop.wav")
alert          = AlertSound(1, 300, false)
array          = [1, 2, 3]
)";
			const std::string_view input = input_str;
			const parser::parse_success_data parse_data = parse(input);
			const parser::lookup_data& lookup_data = parse_data.lookup_data;
			const lang::symbol_table symbols = expect_success_when_resolving_symbols(parse_data.ast.definitions, lookup_data);

			expect_object_in_symbols(symbols, lookup_data, "nothing",        lang::none{},                    search(input, "nothing"),        search(input, "_"));
			expect_object_in_symbols(symbols, lookup_data, "boolean",        lang::boolean{false},            search(input, "boolean"),        search(input, "false"));
			expect_object_in_symbols(symbols, lookup_data, "floating_point", lang::floating_point{3.5},       search(input, "floating_point"), search(input, "3.5"));
			expect_object_in_symbols(symbols, lookup_data, "integer",        lang::integer{123},              search(input, "integer"),        search(input, "123"));
			expect_object_in_symbols(symbols, lookup_data, "level",          lang::level{10},                 search(input, "level"),          search(input, "Level(10)"));
			expect_object_in_symbols(symbols, lookup_data, "font_size",      lang::font_size{30},             search(input, "font_size"),      search(input, "FontSize(30)"));
			expect_object_in_symbols(symbols, lookup_data, "sound_id",       lang::sound_id{7},               search(input, "sound_id"),       search(input, "SoundId(7)"));
			expect_object_in_symbols(symbols, lookup_data, "volume",         lang::volume{300},               search(input, "volume"),         search(input, "Volume(300)"));
			expect_object_in_symbols(symbols, lookup_data, "group",          lang::socket_group{1, 1 , 1, 0}, search(input, "group"),          search(input, "Group(\"RGB\")"));
			expect_object_in_symbols(symbols, lookup_data, "rarity",         lang::rarity::rare,              search(input, "rarity"),         search(input, "rare"));
			expect_object_in_symbols(symbols, lookup_data, "shape",          lang::shape::hexagon,            search(input, "shape"),          search(input, "hexagon"));
			expect_object_in_symbols(symbols, lookup_data, "suit",           lang::suit::brown,               search(input, "suit"),           search(input, "brown"));
			expect_object_in_symbols(symbols, lookup_data, "color",          lang::color{101, 102, 103},      search(input, "color"),          search(input, "RGB(101, 102, 103)"));
			expect_object_in_symbols(symbols, lookup_data, "icon",
				lang::minimap_icon{lang::integer{0}, lang::suit::green, lang::shape::square},
				search(input, "icon"), search(input, "MinimapIcon(0, green, square)"));
			expect_object_in_symbols(symbols, lookup_data, "beam",
				lang::beam_effect{lang::suit::yellow},
				search(input, "beam"), search(input, "Beam(yellow)"));
			expect_object_in_symbols(symbols, lookup_data, "string",         lang::string{"Leather Belt"},    search(input, "string"),         search(input, "\"Leather Belt\""));
			expect_object_in_symbols(symbols, lookup_data, "path",           lang::path{"pop.wav"},           search(input, "path"),           search(input, "Path(\"pop.wav\")"));
			expect_object_in_symbols(symbols, lookup_data, "alert",
				lang::alert_sound{lang::built_in_alert_sound{lang::sound_id{1}, lang::volume{300}, lang::boolean{false}}},
				search(input, "alert"), search(input, "AlertSound(1, 300, false)"));
			expect_object_in_symbols(symbols, lookup_data, "array",
				lang::array_object{
					lang::object{lang::integer{1}, lang::position_tag()},
					lang::object{lang::integer{2}, lang::position_tag()},
					lang::object{lang::integer{3}, lang::position_tag()}},
				search(input, "array"), search(input, "[1, 2, 3]"));
		}

		BOOST_AUTO_TEST_CASE(array_definitions, * ut::description("test that arrays of various sizes are correctly interpreted"))
		{
			const std::string input_str = minimal_input() + R"(
a0 = []
a1 = [1]
a2 = [1, 2]
a3 = [1, 2, 3]
)";
			const std::string_view input = input_str;
			const parser::parse_success_data parse_data = parse(input);
			const parser::lookup_data& lookup_data = parse_data.lookup_data;
			const lang::symbol_table symbols = expect_success_when_resolving_symbols(parse_data.ast.definitions, lookup_data);

			expect_object_in_symbols(symbols, lookup_data, "a0",
				lang::array_object{},
				search(input, "a0"), search(input, "[]"));

			expect_object_in_symbols(symbols, lookup_data, "a1",
				lang::array_object{
					lang::object{lang::integer{1}, lang::position_tag()}},
				search(input, "a1"), search(input, "[1]"));

			expect_object_in_symbols(symbols, lookup_data, "a2",
				lang::array_object{
					lang::object{lang::integer{1}, lang::position_tag()},
					lang::object{lang::integer{2}, lang::position_tag()}},
				search(input, "a2"), search(input, "[1, 2]"));

			expect_object_in_symbols(symbols, lookup_data, "a3",
				lang::array_object{
					lang::object{lang::integer{1}, lang::position_tag()},
					lang::object{lang::integer{2}, lang::position_tag()},
					lang::object{lang::integer{3}, lang::position_tag()}},
				search(input, "a3"), search(input, "[1, 2, 3]"));
		}

		BOOST_AUTO_TEST_CASE(array_subscript, * ut::description("test that array subscript returns correct objects"))
		{
			const std::string input_str = minimal_input() + R"(
arr = [100, 101, 102, 103]
first   = arr[ 0]
second  = arr[ 1]
prelast = arr[-2]
last    = arr[-1]

elem = [1, 2, 3][1]
)";
			const std::string_view input = input_str;
			const parser::parse_success_data parse_data = parse(input);
			const parser::lookup_data& lookup_data = parse_data.lookup_data;
			const lang::symbol_table symbols = expect_success_when_resolving_symbols(parse_data.ast.definitions, lookup_data);

			expect_object_in_symbols(symbols, lookup_data, "first",
				lang::integer{100}, search(input, "first"), search(input, "[ 0]"));

			expect_object_in_symbols(symbols, lookup_data, "second",
				lang::integer{101}, search(input, "second"), search(input, "[ 1]"));

			expect_object_in_symbols(symbols, lookup_data, "prelast",
				lang::integer{102}, search(input, "prelast"), search(input, "[-2]"));

			expect_object_in_symbols(symbols, lookup_data, "last",
				lang::integer{103}, search(input, "last"), search(input, "[-1]"));

			expect_object_in_symbols(symbols, lookup_data, "elem",
				lang::integer{2}, search(input, "elem"), search(input, "[1]"));
		}

		BOOST_AUTO_TEST_CASE(promotions)
		{
			const std::string input_str = minimal_input() + R"(
ItemLevel 10
SocketGroup "RGBW"
{
	SetAlertSound 7
	SetBeam green
	Hide
}

SetAlertSound "error.wav"
Show
)";
			const std::string_view input = input_str;
			const parser::parse_success_data parse_data = parse(input);
			const parser::lookup_data& lookup_data = parse_data.lookup_data;
			const lang::symbol_table symbols = expect_success_when_resolving_symbols(parse_data.ast.definitions, lookup_data);
			const std::vector<lang::filter_block> blocks =
				expect_success_when_building_filter(parse_data.ast.statements, parse_data.lookup_data, symbols);
			BOOST_TEST_REQUIRE(static_cast<int>(blocks.size()) == 2);

			const lang::filter_block& b0 = blocks[0];
			BOOST_TEST(b0.show == false);

			const lang::condition_set& b0_cond = b0.conditions;
			BOOST_TEST(b0_cond.item_level.is_exact());
			BOOST_TEST(b0_cond.item_level.includes(10));
			if (b0_cond.socket_group.has_value())
			{
				const lang::socket_group& sg = (*b0_cond.socket_group).group;
				BOOST_TEST(sg.is_valid());
				BOOST_TEST(sg.r == 1);
				BOOST_TEST(sg.g == 1);
				BOOST_TEST(sg.b == 1);
				BOOST_TEST(sg.w == 1);
			}
			else
			{
				BOOST_ERROR("block 0 has no socket group but it should have");
			}

			const lang::action_set& b0_act = b0.actions;
			if (b0_act.beam_effect.has_value())
			{
				BOOST_TEST((*b0_act.beam_effect).color == lang::suit::green);
				BOOST_TEST((*b0_act.beam_effect).is_temporary == false);
			}
			else
			{
				BOOST_ERROR("block 0 has no beam effect but it should have");
			}

			const lang::filter_block& b1 = blocks[1];
			BOOST_TEST(b1.show == true);

			const lang::action_set& b1_act = b1.actions;
			if (b1_act.alert_sound.has_value())
			{
				if (std::holds_alternative<lang::custom_alert_sound>((*b1_act.alert_sound).sound))
				{
					const auto& sound = std::get<lang::custom_alert_sound>((*b1_act.alert_sound).sound);
					BOOST_TEST(sound.path.value == "error.wav");
				}
				else
				{
					BOOST_ERROR("block 1 has wrong alert sound");
				}
			}
			else
			{
				BOOST_ERROR("block 1 has no alert sound but it should have");
			}
		}

		class alert_sound_fixture : public compiler_success_fixture
		{
		public:
			static
			void test_alert_sound(
				const lang::alert_sound& expected_alert_sound,
				const lang::alert_sound& actual_alert_sound)
			{
				std::visit(utility::visitor{
					[&](const lang::built_in_alert_sound& expected)
					{
						if (!std::holds_alternative<lang::built_in_alert_sound>(actual_alert_sound.sound))
							BOOST_ERROR("expected built-in alert sound but got different");

						const auto& actual = std::get<lang::built_in_alert_sound>(actual_alert_sound.sound);
						BOOST_TEST(actual.id.value == expected.id.value);
						BOOST_TEST((actual.volume == expected.volume));
						BOOST_TEST(actual.is_positional.value == expected.is_positional.value);
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
			void expect_alert_sound(
				std::string expression,
				const lang::alert_sound& expected_value)
			{
				const std::string input_str = minimal_input() + "SetAlertSound " + expression + "\nShow";
				const std::string_view input = input_str;
				const parser::parse_success_data parse_data = parse(input);
				const parser::lookup_data& lookup_data = parse_data.lookup_data;
				const lang::symbol_table symbols = expect_success_when_resolving_symbols(parse_data.ast.definitions, lookup_data);
				const std::vector<lang::filter_block> blocks =
					expect_success_when_building_filter(parse_data.ast.statements, parse_data.lookup_data, symbols);
				BOOST_TEST_REQUIRE(static_cast<int>(blocks.size()) == 1);
				const lang::filter_block& block = blocks[0];
				BOOST_TEST(block.show == true);
				const std::optional<lang::alert_sound>& maybe_alert_sound = block.actions.alert_sound;
				BOOST_TEST_REQUIRE(maybe_alert_sound.has_value());
				const lang::alert_sound& alert_sound = *maybe_alert_sound;
				test_alert_sound(expected_value, alert_sound);
			}
		};

		BOOST_FIXTURE_TEST_SUITE(alert_sound_suite, alert_sound_fixture)

			using namespace lang;
			// note: some test case names end in _ because their name causes name conflicts with lang:: types

			BOOST_AUTO_TEST_CASE(promoted_integer)
			{
				expect_alert_sound("1", alert_sound(built_in_alert_sound(integer{1})));
			}

			BOOST_AUTO_TEST_CASE(promoted_sound_id)
			{
				expect_alert_sound("SoundId(1)", alert_sound(built_in_alert_sound(sound_id{integer{1}})));
			}

			BOOST_AUTO_TEST_CASE(integer_)
			{
				expect_alert_sound("AlertSound(1)", alert_sound(built_in_alert_sound(integer{1})));
			}

			BOOST_AUTO_TEST_CASE(sound_id_)
			{
				expect_alert_sound("AlertSound(SoundId(1))", alert_sound(built_in_alert_sound(sound_id{integer{1}})));
			}

			BOOST_AUTO_TEST_CASE(integer_and_integer)
			{
				expect_alert_sound("AlertSound(1, 300)", alert_sound(built_in_alert_sound(sound_id{integer{1}}, volume{300})));
			}

			BOOST_AUTO_TEST_CASE(integer_and_boolean)
			{
				expect_alert_sound("AlertSound(1, true)", alert_sound(built_in_alert_sound(sound_id{integer{1}}, boolean{true})));
			}

			BOOST_AUTO_TEST_CASE(integer_and_integer_and_boolean)
			{
				expect_alert_sound("AlertSound(1, 200, true)", alert_sound(built_in_alert_sound(sound_id{integer{1}}, volume{200}, boolean{true})));
			}

			BOOST_AUTO_TEST_CASE(promoted_string)
			{
				expect_alert_sound("\"pop.wav\"", alert_sound(custom_alert_sound(string{"pop.wav"})));
			}

			BOOST_AUTO_TEST_CASE(promoted_path)
			{
				expect_alert_sound("Path(\"pop.wav\")", alert_sound(custom_alert_sound(path{"pop.wav"})));
			}

			BOOST_AUTO_TEST_CASE(string_)
			{
				expect_alert_sound("AlertSound(\"pop.wav\")", alert_sound(custom_alert_sound(string{"pop.wav"})));
			}

			BOOST_AUTO_TEST_CASE(path_)
			{
				expect_alert_sound("AlertSound(Path(\"pop.wav\"))", alert_sound(custom_alert_sound(path{"pop.wav"})));
			}

		BOOST_AUTO_TEST_SUITE_END()

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
