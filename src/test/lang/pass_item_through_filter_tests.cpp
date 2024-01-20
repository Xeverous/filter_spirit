#include <fs/lang/item_filter.hpp>
#include <fs/parser/parser.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/log/string_logger.hpp>

#include <boost/test/unit_test.hpp>

#include <string>
#include <string_view>

namespace ut = boost::unit_test;
namespace tt = boost::test_tools;

namespace fs::test {

namespace {

lang::socket_info make_socket_info(std::string_view str)
{
	std::optional<lang::socket_info> sockets = lang::to_socket_info(str);
	BOOST_TEST_REQUIRE(sockets.has_value());
	return sockets.value();
}

lang::item_filter parse_real_filter(std::string_view source)
{
	std::variant<parser::parsed_real_filter, parser::parse_failure_data> result = parser::parse_real_filter(source);
	BOOST_TEST_REQUIRE(std::holds_alternative<parser::parsed_real_filter>(result));

	const auto& parsed_filter = std::get<parser::parsed_real_filter>(result);
	compiler::diagnostics_store diagnostics;
	boost::optional<lang::item_filter> filter = compiler::compile_real_filter(compiler::settings{}, parsed_filter.ast, diagnostics);
	if (diagnostics.has_errors() || !filter) {
		log::string_logger log;
		diagnostics.output_diagnostics(parsed_filter.metadata, log);
		BOOST_FAIL("Filter compilation failed: " << log.str());
	}
	BOOST_TEST_REQUIRE(filter.has_value());
	return filter.value();
}

[[nodiscard]] bool
test_sockets_condition_impl(std::string source, lang::socket_info sockets)
{
	const std::string filter_source = "Show\n\t" + source + "\n\tPlayEffect Yellow";
	const lang::item_filter filter = parse_real_filter(filter_source);
	lang::item itm;
	itm.sockets = sockets;
	const lang::item_filtering_result result = lang::pass_item_through_filter(itm, filter, 1);
	const bool item_caught = result.style.play_effect.has_value();
	BOOST_TEST_MESSAGE("checking " << source << " against " << lang::to_string(sockets) << " # " << (item_caught ? "true" : "false"));
	return item_caught;
}

// functions return whether the item matches a block with sockets condition
[[nodiscard]] bool
test_sockets_condition(std::string source, lang::socket_info sockets)
{
	return test_sockets_condition_impl("Sockets     " + source, sockets);
}

[[nodiscard]] bool
test_socket_group_condition(std::string source, lang::socket_info sockets)
{
	return test_sockets_condition_impl("SocketGroup " + source, sockets);
}

} // namespace

BOOST_AUTO_TEST_SUITE(socket_spec_suite)

	BOOST_AUTO_TEST_CASE(no_sockets)
	{
		const lang::socket_info sockets = {};
		BOOST_TEST(!test_sockets_condition(     "< 0    ", sockets));
		BOOST_TEST( test_sockets_condition(     "< 1    ", sockets));
		BOOST_TEST( test_sockets_condition(     "< 1RGB ", sockets));
		BOOST_TEST( test_sockets_condition(     "<  RGB ", sockets));
		BOOST_TEST( test_sockets_condition(     "<= 1"   , sockets));
		BOOST_TEST( test_sockets_condition(     "<= 1RGB", sockets));
		BOOST_TEST( test_sockets_condition(     "<=  RGB", sockets));
		BOOST_TEST( test_socket_group_condition("< 1    ", sockets));
		BOOST_TEST( test_socket_group_condition("< 1RGB ", sockets));
		BOOST_TEST( test_socket_group_condition("<  RGB ", sockets));
		BOOST_TEST( test_socket_group_condition("<= 1"   , sockets));
		BOOST_TEST( test_socket_group_condition("<= 1RGB", sockets));
		BOOST_TEST( test_socket_group_condition("<=  RGB", sockets));

		BOOST_TEST(!test_socket_group_condition("<  0   ", sockets));
		BOOST_TEST( test_socket_group_condition("<= 0   ", sockets));
		BOOST_TEST( test_socket_group_condition("   0   ", sockets));
		BOOST_TEST( test_socket_group_condition("== 0   ", sockets));

		BOOST_TEST(!test_sockets_condition(     ">  0   ", sockets));
		BOOST_TEST(!test_sockets_condition(     ">  0R  ", sockets));
		BOOST_TEST(!test_socket_group_condition(">  0   ", sockets));
		BOOST_TEST(!test_socket_group_condition(">  0R  ", sockets));

		BOOST_TEST( test_sockets_condition(     ">= 0   ", sockets));
		BOOST_TEST(!test_sockets_condition(     ">= 0R  ", sockets));
		BOOST_TEST( test_socket_group_condition(">= 0   ", sockets));
		BOOST_TEST(!test_socket_group_condition(">= 0R  ", sockets));
	}

	BOOST_AUTO_TEST_CASE(_4L)
	{
		lang::socket_info sockets = make_socket_info("B-B R R");
		BOOST_TEST( test_socket_group_condition(">  1R", sockets));
		BOOST_TEST(!test_socket_group_condition(">  2R", sockets));
		BOOST_TEST( test_socket_group_condition(">= 1R", sockets));
		BOOST_TEST(!test_socket_group_condition(">= 2R", sockets));

		sockets = make_socket_info("B B R-R");
		BOOST_TEST( test_socket_group_condition(">  1R", sockets));
		BOOST_TEST( test_socket_group_condition(">  2R", sockets));
		BOOST_TEST( test_socket_group_condition(">= 1R", sockets));
		BOOST_TEST( test_socket_group_condition(">= 2R", sockets));
	}

	BOOST_AUTO_TEST_CASE(unintuitive_4L)
	{
		const lang::socket_info sockets = make_socket_info("R-R-R-G");
		BOOST_TEST( test_sockets_condition(     "<  5RR", sockets));
		BOOST_TEST(!test_sockets_condition(     "<= 5RR", sockets));
		BOOST_TEST( test_socket_group_condition("<  5RR", sockets));
		BOOST_TEST(!test_socket_group_condition("<= 5RR", sockets));
	}

	BOOST_AUTO_TEST_CASE(unintuitive_6L)
	{
		const lang::socket_info sockets = make_socket_info("R-R-R-R-G-B");
		BOOST_TEST( test_sockets_condition(     "<  5GGG", sockets));
		BOOST_TEST(!test_sockets_condition(     "<= 5GGG", sockets));
		BOOST_TEST( test_socket_group_condition("<  5GGG", sockets));
		BOOST_TEST(!test_socket_group_condition("<= 5GGG", sockets));
	}

	void test_5ggg_impl(
		bool (&f)(std::string, lang::socket_info),
		bool less, bool less_eq, bool eq_soft, bool eq_hard, bool greater_eq, bool greater,
		lang::socket_info sockets)
	{
		BOOST_TEST(f("<  5GGG", sockets) == less);
		BOOST_TEST(f("<= 5GGG", sockets) == less_eq);
		BOOST_TEST(f("   5GGG", sockets) == eq_soft);
		BOOST_TEST(f("== 5GGG", sockets) == eq_hard);
		BOOST_TEST(f(">= 5GGG", sockets) == greater_eq);
		BOOST_TEST(f(">  5GGG", sockets) == greater);
	}

	void test_5ggg_sockets(
		bool less, bool less_eq, bool eq_soft, bool eq_hard, bool greater_eq, bool greater,
		lang::socket_info sockets)
	{
		test_5ggg_impl(test_sockets_condition, less, less_eq, eq_soft, eq_hard, greater_eq, greater, sockets);
	}

	void test_5ggg_socket_group(
		bool less, bool less_eq, bool eq_soft, bool eq_hard, bool greater_eq, bool greater,
		lang::socket_info sockets)
	{
		test_5ggg_impl(test_socket_group_condition, less, less_eq, eq_soft, eq_hard, greater_eq, greater, sockets);
	}

	BOOST_AUTO_TEST_CASE(_5s_5L_3G)
	{
		const lang::socket_info sockets = make_socket_info("W-G-G-G-R");
		test_5ggg_sockets     (false, true, true, true, true, false, sockets);
		test_5ggg_socket_group(false, true, true, true, true, false, sockets);
	}

	BOOST_AUTO_TEST_CASE(_5s_5L_4G)
	{
		const lang::socket_info sockets = make_socket_info("G-G-W-G-G");
		test_5ggg_sockets     (false, false, true, false, true, true, sockets);
		test_5ggg_socket_group(false, false, true, false, true, true, sockets);
	}

	BOOST_AUTO_TEST_CASE(_6s_5L_2G_and_1G)
	{
		const lang::socket_info sockets = make_socket_info("W-G-B-G-R G");
		test_5ggg_sockets     (false, false, false, false, true,  true,  sockets);
		test_5ggg_socket_group(true,  true,  false, false, false, false, sockets);
	}

	BOOST_AUTO_TEST_CASE(_6s_5L_3G_and_1G)
	{
		const lang::socket_info sockets = make_socket_info("W-G-G-G-R G");
		test_5ggg_sockets     (false, false, false, false, true, true,  sockets);
		test_5ggg_socket_group(true,  true,  true,  true,  true, false, sockets);
	}

	BOOST_AUTO_TEST_CASE(_6L_3G)
	{
		const lang::socket_info sockets = make_socket_info("W-G-B-G-R-G");
		test_5ggg_sockets     (false, false, false, false, true, true, sockets);
		test_5ggg_socket_group(false, false, false, false, true, true, sockets);
	}

	BOOST_AUTO_TEST_CASE(_6L_4G)
	{
		const lang::socket_info sockets = make_socket_info("G-G-B-G-R-G");
		test_5ggg_sockets     (false, false, false, false, true, true, sockets);
		test_5ggg_socket_group(false, false, false, false, true, true, sockets);
	}

	BOOST_AUTO_TEST_CASE(less_than_4RG)
	{
		const std::string condition = "<  4RG";

		lang::socket_info sockets = make_socket_info("B-B-B");
		BOOST_TEST( test_sockets_condition(     condition, sockets));
		BOOST_TEST( test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("G-G-G-G");
		BOOST_TEST( test_sockets_condition(     condition, sockets));
		BOOST_TEST( test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("R-R-R-R");
		BOOST_TEST( test_sockets_condition(     condition, sockets));
		BOOST_TEST( test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("R-G-B-B");
		BOOST_TEST(!test_sockets_condition(     condition, sockets));
		BOOST_TEST(!test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("R-G B-B");
		BOOST_TEST(!test_sockets_condition(     condition, sockets));
		BOOST_TEST( test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("B-B-B-B G-R");
		BOOST_TEST(!test_sockets_condition(     condition, sockets));
		BOOST_TEST( test_socket_group_condition(condition, sockets));
	}

	BOOST_AUTO_TEST_CASE(more_than_4RG)
	{
		const std::string condition = ">  4RG";

		lang::socket_info sockets = make_socket_info("B-B-B-B-B");
		BOOST_TEST( test_sockets_condition(     condition, sockets));
		BOOST_TEST( test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("R-R");
		BOOST_TEST( test_sockets_condition(     condition, sockets));
		BOOST_TEST( test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("R R");
		BOOST_TEST( test_sockets_condition(     condition, sockets));
		BOOST_TEST(!test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("G-G");
		BOOST_TEST( test_sockets_condition(     condition, sockets));
		BOOST_TEST( test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("G G");
		BOOST_TEST( test_sockets_condition(     condition, sockets));
		BOOST_TEST(!test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("R-G");
		BOOST_TEST(!test_sockets_condition(     condition, sockets));
		BOOST_TEST(!test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("R-G-B-B");
		BOOST_TEST(!test_sockets_condition(     condition, sockets));
		BOOST_TEST(!test_socket_group_condition(condition, sockets));

		sockets = make_socket_info("R-G-B-B B");
		BOOST_TEST( test_sockets_condition(     condition, sockets));
		BOOST_TEST(!test_socket_group_condition(condition, sockets));
	}

BOOST_AUTO_TEST_SUITE_END()

} // namespace fs::test
