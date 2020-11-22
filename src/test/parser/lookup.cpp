#include <fs/parser/parser.hpp>

#include <boost/test/unit_test.hpp>

#include <string_view>

namespace fs::test {

BOOST_AUTO_TEST_SUITE(parser_success_suite)

	BOOST_AUTO_TEST_CASE(text_positions)
	{
		const std::string_view input = "AAA\nBB\n\nC";
		const parser::line_lookup ll(input.data(), input.data() + input.size());

		BOOST_TEST(ll.num_lines() == 4u);

		BOOST_TEST(ll.get_line(0) == "AAA");
		BOOST_TEST(ll.get_line(1) == "BB");
		BOOST_TEST(ll.get_line(2) == "");
		BOOST_TEST(ll.get_line(3) == "C");
		BOOST_TEST(ll.get_line(4) == "");
		BOOST_TEST(ll.get_line(5) == "");

		const parser::text_position pos_0_0 = ll.text_position_for(input.data());
		BOOST_TEST(pos_0_0.line_number == 0);
		BOOST_TEST(pos_0_0.pos == 0);

		const parser::text_position pos_1_1 = ll.text_position_for(input.data() + 5);
		BOOST_TEST(pos_1_1.line_number == 1);
		BOOST_TEST(pos_1_1.pos == 1);

		const parser::text_position pos_3_0 = ll.text_position_for(input.data() + 8);
		BOOST_TEST(pos_3_0.line_number == 3);
		BOOST_TEST(pos_3_0.pos == 0);

		const parser::text_range r = ll.text_range_for(input);
		BOOST_TEST(r.first.line_number == 0);
		BOOST_TEST(r.first.pos == 0);
		BOOST_TEST(r.last.line_number == 3);
		BOOST_TEST(r.last.pos == 1);
	}

	BOOST_AUTO_TEST_CASE(text_range_empty)
	{
		const std::string_view input = "";
		const parser::line_lookup ll(input.data(), input.data() + input.size());

		BOOST_TEST(ll.num_lines() == 0u);

		// empty range invariant: first == last
		const parser::text_range r = ll.text_range_for(input);
		BOOST_TEST(r.first.line_number == 0);
		BOOST_TEST(r.first.pos == 0);
		BOOST_TEST(r.last.line_number == 0);
		BOOST_TEST(r.last.pos == 0);
	}

	BOOST_AUTO_TEST_CASE(text_range_one)
	{
		const std::string_view input = "1";
		const parser::line_lookup ll(input.data(), input.data() + input.size());

		BOOST_TEST(ll.num_lines() == 1u);

		const parser::text_range r = ll.text_range_for(input);
		BOOST_TEST(r.first.line_number == 0);
		BOOST_TEST(r.first.pos == 0);
		BOOST_TEST(r.last.line_number == 0);
		BOOST_TEST(r.last.pos == 1);
	}

	BOOST_AUTO_TEST_CASE(text_range_lf_end)
	{
		const std::string_view input = "1\n";
		const parser::line_lookup ll(input.data(), input.data() + input.size());

		BOOST_TEST(ll.num_lines() == 1u);

		const parser::text_range r = ll.text_range_for(input);
		BOOST_TEST(r.first.line_number == 0);
		BOOST_TEST(r.first.pos == 0);
		BOOST_TEST(r.last.line_number == 1);
		BOOST_TEST(r.last.pos == 0);
	}

BOOST_AUTO_TEST_SUITE_END()

}
