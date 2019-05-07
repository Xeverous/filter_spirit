#include "fst/common/print_type.hpp"
#include "fst/common/test_fixtures.hpp"
#include "fst/common/string_operations.hpp"

#include "fs/compiler/compiler.hpp"
#include "fs/compiler/print_error.hpp"
#include "fs/compiler/detail/filter_builder.hpp"
#include "fs/log/buffered_logger.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string_view>
#include <utility>

namespace ut = boost::unit_test;

namespace fst
{

void expect_object_in_map(
	const fs::lang::constants_map& map,
	const fs::parser::lookup_data& lookup_data,
	const std::string& name,
	const fs::lang::object_variant& value,
	std::string_view name_origin,
	std::string_view value_origin)
{
	const auto it = map.find(name);
	if (it == map.end())
	{
		BOOST_ERROR("expected " << name << " in constants map");
		return;
	}

	const fs::lang::named_object& named_object = it->second;
	const fs::lang::object& object = named_object.object_instance;
	if (object.value != value)
	{
		const std::string_view all_code = lookup_data.get_view_of_whole_content();
		BOOST_ERROR("expected " << fs::lang::type_of_object(value) << " but got " << fs::lang::type_of_object(object.value) << "\n"
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
		fs::lang::constants_map expect_success_when_resolving_constants(
			const std::vector<fs::parser::ast::constant_definition>& defs,
			const fs::parser::lookup_data& lookup_data)
		{
			const std::variant<fs::lang::constants_map, fs::compiler::error::error_variant> map_or_error =
				resolve_constants(defs);

			if (std::holds_alternative<fs::compiler::error::error_variant>(map_or_error))
			{
				const auto& error = std::get<fs::compiler::error::error_variant>(map_or_error);
				fs::log::buffered_logger logger;
				fs::compiler::print_error(error, lookup_data, logger);
				const auto log = logger.flush_out();
				BOOST_FAIL("resolve_constants failed but should not:\n" << log);
			}

			return std::get<fs::lang::constants_map>(std::move(map_or_error));
		}

		static
		std::vector<fs::lang::filter_block> expect_succes_when_building_filter(
			const std::vector<fs::parser::ast::statement>& top_level_statements,
			const fs::parser::lookup_data& lookup_data,
			const fs::lang::constants_map& map)
		{
			std::variant<std::vector<fs::lang::filter_block>, fs::compiler::error::error_variant> result =
				fs::compiler::detail::filter_builder::build_filter(
					top_level_statements, map, fs::itemdata::item_price_data{});

			if (std::holds_alternative<fs::compiler::error::error_variant>(result))
			{
				const auto& error = std::get<fs::compiler::error::error_variant>(result);
				fs::log::buffered_logger logger;
				fs::compiler::print_error(error, lookup_data, logger);
				const auto log = logger.flush_out();
				BOOST_FAIL("building filter blocks failed but should not:\n" << log);
			}

			return std::get<std::vector<fs::lang::filter_block>>(std::move(result));
		}
	};

	BOOST_FIXTURE_TEST_SUITE(compiler_success_suite, compiler_success_fixture,
		* ut::depends_on("compiler_suite/minimal_input_resolve_constants"))

		BOOST_AUTO_TEST_CASE(all_possible_constants,
			* ut::description("test that all types can be used as constant"))
		{
			const std::string input_str = minimal_input() + R"(
const boolean        = false
const floating_point = 3.5
const integer        = 123
const level          = Level(10)
const font_size      = FontSize(30)
const sound_id       = SoundId(7)
const volume         = Volume(300)
const group          = Group("RGB")
const rarity         = rare
const shape          = hexagon
const suit           = brown
const color          = RGB(101, 102, 103)
const icon           = MinimapIcon(0, green, square)
const beam           = Beam(yellow)
const string         = "Leather Belt"
const path           = Path("pop.wav")
const alert          = AlertSound(1, 300, false)
const array          = [1, 2, 3]
# const dict           = { "a": 1, "b": 2 }
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::parser::lookup_data& lookup_data = parse_data.lookup_data;
			const fs::lang::constants_map map = expect_success_when_resolving_constants(parse_data.ast.constant_definitions, lookup_data);

			expect_object_in_map(map, lookup_data, "boolean",        fs::lang::boolean{false},            search(input, "boolean"),        search(input, "false"));
			expect_object_in_map(map, lookup_data, "floating_point", fs::lang::floating_point{3.5},       search(input, "floating_point"), search(input, "3.5"));
			expect_object_in_map(map, lookup_data, "integer",        fs::lang::integer{123},              search(input, "integer"),        search(input, "123"));
			expect_object_in_map(map, lookup_data, "level",          fs::lang::level{10},                 search(input, "level"),          search(input, "Level(10)"));
			expect_object_in_map(map, lookup_data, "font_size",      fs::lang::font_size{30},             search(input, "font_size"),      search(input, "FontSize(30)"));
			expect_object_in_map(map, lookup_data, "sound_id",       fs::lang::sound_id{7},               search(input, "sound_id"),       search(input, "SoundId(7)"));
			expect_object_in_map(map, lookup_data, "volume",         fs::lang::volume{300},               search(input, "volume"),         search(input, "Volume(300)"));
			expect_object_in_map(map, lookup_data, "group",          fs::lang::socket_group{1, 1 , 1, 0}, search(input, "group"),          search(input, "Group(\"RGB\")"));
			expect_object_in_map(map, lookup_data, "rarity",         fs::lang::rarity::rare,              search(input, "rarity"),         search(input, "rare"));
			expect_object_in_map(map, lookup_data, "shape",          fs::lang::shape::hexagon,            search(input, "shape"),          search(input, "hexagon"));
			expect_object_in_map(map, lookup_data, "suit",           fs::lang::suit::brown,               search(input, "suit"),           search(input, "brown"));
			expect_object_in_map(map, lookup_data, "color",          fs::lang::color{101, 102, 103},      search(input, "color"),          search(input, "RGB(101, 102, 103)"));
			expect_object_in_map(map, lookup_data, "icon",
				fs::lang::minimap_icon{fs::lang::integer{0}, fs::lang::suit::green, fs::lang::shape::square},
				search(input, "icon"), search(input, "MinimapIcon(0, green, square)"));
			expect_object_in_map(map, lookup_data, "beam",
				fs::lang::beam_effect{fs::lang::suit::yellow},
				search(input, "beam"), search(input, "Beam(yellow)"));
			expect_object_in_map(map, lookup_data, "string",         fs::lang::string{"Leather Belt"},    search(input, "string"),         search(input, "\"Leather Belt\""));
			expect_object_in_map(map, lookup_data, "path",           fs::lang::path{"pop.wav"},           search(input, "path"),           search(input, "Path(\"pop.wav\")"));
			expect_object_in_map(map, lookup_data, "alert",
				fs::lang::alert_sound{fs::lang::built_in_alert_sound{fs::lang::sound_id{1}, fs::lang::volume{300}, fs::lang::boolean{false}}},
				search(input, "alert"), search(input, "AlertSound(1, 300, false)"));
			expect_object_in_map(map, lookup_data, "array",
				fs::lang::array_object{
					fs::lang::object{fs::lang::integer{1}, fs::lang::position_tag()},
					fs::lang::object{fs::lang::integer{2}, fs::lang::position_tag()},
					fs::lang::object{fs::lang::integer{3}, fs::lang::position_tag()}},
				search(input, "array"), search(input, "[1, 2, 3]"));
			/* expect_object_in_map(map, lookup_data, "dict",
				fs::lang::dict_object{
					{"a", fs::lang::object{fs::lang::integer{1}, fs::lang::position_tag()}},
					{"b", fs::lang::object{fs::lang::integer{2}, fs::lang::position_tag()}}},
				search(input, "dict"), search(input, R"({ "a": 1, "b": 2 })")); */
		}

		BOOST_AUTO_TEST_CASE(array_definitions, * ut::description("test that arrays of various sizes are correctly interpreted"))
		{
			const std::string input_str = minimal_input() + R"(
const a0 = []
const a1 = [1]
const a2 = [1, 2]
const a3 = [1, 2, 3]
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::parser::lookup_data& lookup_data = parse_data.lookup_data;
			const fs::lang::constants_map map = expect_success_when_resolving_constants(parse_data.ast.constant_definitions, lookup_data);

			expect_object_in_map(map, lookup_data, "a0",
				fs::lang::array_object{},
				search(input, "a0"), search(input, "[]"));

			expect_object_in_map(map, lookup_data, "a1",
				fs::lang::array_object{
					fs::lang::object{fs::lang::integer{1}, fs::lang::position_tag()}},
				search(input, "a1"), search(input, "[1]"));

			expect_object_in_map(map, lookup_data, "a2",
				fs::lang::array_object{
					fs::lang::object{fs::lang::integer{1}, fs::lang::position_tag()},
					fs::lang::object{fs::lang::integer{2}, fs::lang::position_tag()}},
				search(input, "a2"), search(input, "[1, 2]"));

			expect_object_in_map(map, lookup_data, "a3",
				fs::lang::array_object{
					fs::lang::object{fs::lang::integer{1}, fs::lang::position_tag()},
					fs::lang::object{fs::lang::integer{2}, fs::lang::position_tag()},
					fs::lang::object{fs::lang::integer{3}, fs::lang::position_tag()}},
				search(input, "a3"), search(input, "[1, 2, 3]"));
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
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::parser::lookup_data& lookup_data = parse_data.lookup_data;
			const fs::lang::constants_map map = expect_success_when_resolving_constants(parse_data.ast.constant_definitions, lookup_data);
			const std::vector<fs::lang::filter_block> blocks =
				expect_succes_when_building_filter(parse_data.ast.statements, parse_data.lookup_data, map);
			BOOST_TEST_REQUIRE(static_cast<int>(blocks.size()) == 2);

			const fs::lang::filter_block& b0 = blocks[0];
			BOOST_TEST(b0.show == false);

			const fs::lang::condition_set& b0_cond = b0.conditions;
			BOOST_TEST(b0_cond.item_level.is_exact());
			BOOST_TEST(b0_cond.item_level.includes(10));
			if (b0_cond.socket_group.has_value())
			{
				const fs::lang::socket_group& sg = *b0_cond.socket_group;
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

			const fs::lang::action_set& b0_act = b0.actions;
			if (b0_act.beam_effect.has_value())
			{
				BOOST_TEST((*b0_act.beam_effect).color == fs::lang::suit::green);
				BOOST_TEST((*b0_act.beam_effect).is_temporary == false);
			}
			else
			{
				BOOST_ERROR("block 0 has no beam effect but it should have");
			}

			const fs::lang::filter_block& b1 = blocks[1];
			BOOST_TEST(b1.show == true);

			const fs::lang::action_set& b1_act = b1.actions;
			if (b1_act.alert_sound.has_value())
			{
				if (std::holds_alternative<fs::lang::custom_alert_sound>((*b1_act.alert_sound).sound))
				{
					const auto& sound = std::get<fs::lang::custom_alert_sound>((*b1_act.alert_sound).sound);
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

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
