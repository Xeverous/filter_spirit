#include "fs/compiler/compiler.hpp"
#include "fs/log/buffered_logger.hpp"
#include "fs/print/generic.hpp"

#include "fsut/compiler/common.hpp"
#include "fsut/parser/common.hpp"
#include "fsut/lang/print_type.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string>
#include <string_view>
#include <functional>

namespace ut = boost::unit_test;
namespace tt = boost::test_tools;

using range_type = fs::parser::range_type;
using iterator_type = fs::parser::iterator_type;

range_type search(range_type input, std::string_view pattern)
{
	BOOST_TEST_REQUIRE(!input.empty(), "test is written incorrectly, input is empty");
	BOOST_TEST_REQUIRE(!pattern.empty(), "test is written incorrectly, pattern is empty");

	/*
	 * We use this function to search within input for the text that causes compiler error.
	 * Since we always want to get that text in order to compare it with error description,
	 * a failed search should be flagged as internal test error.
	 */
	using searcher_t = std::default_searcher<std::string_view::const_iterator>;
	searcher_t searcher(pattern.begin(), pattern.end());
	const auto pair = searcher(input.begin(), input.end());
	BOOST_TEST_REQUIRE((pair.first != input.end()), "test is written incorrectly, search within tested input code failed");
	return range_type(pair.first, pair.second);
}

std::string range_info_to_string(
	range_type content_range,
	range_type indicated_range)
{
	const int line_number = fs::print::count_line_number(content_range.begin(), indicated_range.begin());
	fs::buffered_logger logger;
	fs::print::print_line_number(logger, line_number);
	fs::print::print_line_with_indicator(logger, content_range, indicated_range);
	return logger.flush_out();
}

tt::predicate_result compare_ranges(
	range_type expected,
	range_type actual,
	range_type input_range /* <- for pretty-printing */)
{
	if (expected.empty())
		BOOST_FAIL("test written incorrectly: an empty range should never be expected");

	if (expected == actual)
		return true;

	tt::predicate_result result(false);
	result.message()
		<< "EXPECTED:\n" << range_info_to_string(input_range, expected)
		<< "ACTUAL:\n" << range_info_to_string(input_range, actual);
	return result;
}

BOOST_FIXTURE_TEST_SUITE(compiler_suite, fsut::compiler::compiler_fixture)

	BOOST_AUTO_TEST_CASE(minimal_input)
	{
		const fs::parser::parse_data parse_data = parse(fsut::parser::minimal_input());
		const std::variant<fs::lang::constants_map, fs::compiler::error::error_variant> map_or_error =
			resolve_constants(parse_data.ast.constant_definitions);
		BOOST_TEST_REQUIRE(std::holds_alternative<fs::lang::constants_map>(map_or_error));
		const auto& map = std::get<fs::lang::constants_map>(map_or_error);
		BOOST_TEST(map.empty());
	}

	/*
	 * tests that check for correct compiler errors for common mistakes
	 *
	 * - parse should succeed
	 * - compilation step(s) should fail
	 *   - check the the error variant holds correct error type
	 *   - check that error information points to relevant place in input
	 */
	class compiler_error_fixture : public fsut::compiler::compiler_fixture
	{
	protected:
		template <typename T>
		T expect_error_when_resolving_constants(const std::vector<fs::parser::ast::constant_definition>& defs)
		{
			const std::variant<fs::lang::constants_map, fs::compiler::error::error_variant> map_or_error =
				resolve_constants(defs);
			BOOST_TEST_REQUIRE(std::holds_alternative<fs::compiler::error::error_variant>(map_or_error));
			const auto& error = std::get<fs::compiler::error::error_variant>(map_or_error);
			BOOST_TEST_REQUIRE(std::holds_alternative<T>(error));
			return std::get<T>(error);
		}
	};

	BOOST_FIXTURE_TEST_SUITE(compiler_error_suite, compiler_error_fixture, * ut::depends_on("compiler_suite/minimal_input"))
		namespace error = fs::compiler::error;

		BOOST_AUTO_TEST_CASE(name_already_exists)
		{
			const std::string input_str = fsut::parser::minimal_input() + R"(
const some_var = 0
const xyz = 1
const some_other_var = 2
const xyz = 3
)";
			const auto input = std::string_view(input_str.data(), input_str.size());
			const fs::parser::parse_data parse_data = parse(input);
			const auto error_desc = expect_error_when_resolving_constants<error::name_already_exists>(parse_data.ast.constant_definitions);

			const range_type input_range(input.begin(), input.end());
			const std::string_view pattern = "xyz";

			const range_type expected_place_of_original_name = search(input_range, pattern);
			const range_type reported_place_of_original_name = parse_data.lookup_data.position_of(error_desc.place_of_original_name);
			BOOST_TEST(compare_ranges(expected_place_of_original_name, reported_place_of_original_name, input_range));

			const range_type expected_place_of_duplicated_name = search(range_type(expected_place_of_original_name.end(), input_range.end()), pattern);
			const range_type reported_place_of_duplicated_name = parse_data.lookup_data.position_of(error_desc.place_of_duplicated_name);
			BOOST_TEST(compare_ranges(expected_place_of_duplicated_name, reported_place_of_duplicated_name, input_range));
		}

		BOOST_AUTO_TEST_CASE(no_such_name)
		{
			const std::string input_str = fsut::parser::minimal_input() + R"(
const abc = 0
const xyz = non_existent_obj
)";
			const auto input = std::string_view(input_str.data(), input_str.size());
			const fs::parser::parse_data parse_data = parse(input);
			const auto error_desc = expect_error_when_resolving_constants<error::no_such_name>(parse_data.ast.constant_definitions);

			const range_type input_range(input.begin(), input.end());
			const range_type expected_place_of_name = search(input_range, "non_existent_obj");
			const range_type reported_place_of_name = parse_data.lookup_data.position_of(error_desc.place_of_name);
			BOOST_TEST(compare_ranges(expected_place_of_name, reported_place_of_name, input_range));
		}

		BOOST_AUTO_TEST_CASE(no_such_function)
		{
			const std::string input_str = fsut::parser::minimal_input() + R"(
const abc = non_existent_func(0)
)";
			const auto input = std::string_view(input_str.data(), input_str.size());
			const fs::parser::parse_data parse_data = parse(input);
			const auto error_desc = expect_error_when_resolving_constants<error::no_such_function>(parse_data.ast.constant_definitions);

			const range_type input_range(input.begin(), input.end());
			const range_type expected_place_of_name = search(input_range, "non_existent_func");
			const range_type reported_place_of_name = parse_data.lookup_data.position_of(error_desc.place_of_name);
			BOOST_TEST(compare_ranges(expected_place_of_name, reported_place_of_name, input_range));
		}

		BOOST_AUTO_TEST_CASE(invalid_amount_of_arguments)
		{
			const std::string input_str = fsut::parser::minimal_input() + R"(
const color = Path(11, 22)
)";
			const auto input = std::string_view(input_str.data(), input_str.size());
			const fs::parser::parse_data parse_data = parse(input);
			const auto error_desc = expect_error_when_resolving_constants<error::invalid_amount_of_arguments>(parse_data.ast.constant_definitions);

			const range_type input_range(input.begin(), input.end());
			const range_type expected_place_of_arguments = search(input_range, "11, 22");
			const range_type reported_place_of_arguments = parse_data.lookup_data.position_of(error_desc.place_of_arguments);
			BOOST_TEST(compare_ranges(expected_place_of_arguments, reported_place_of_arguments, input_range));
		}

		BOOST_AUTO_TEST_CASE(type_mismatch)
		{
			const std::string input_str = fsut::parser::minimal_input() + R"(
const color = RGB(11, 22, "33")
)";
			const auto input = std::string_view(input_str.data(), input_str.size());
			const fs::parser::parse_data parse_data = parse(input);
			const auto error_desc = expect_error_when_resolving_constants<error::type_mismatch>(parse_data.ast.constant_definitions);

			BOOST_TEST(error_desc.expected_type.is_array == false);
			BOOST_TEST(error_desc.expected_type.type == fs::lang::single_object_type::integer);

			BOOST_TEST(error_desc.actual_type.is_array == false);
			BOOST_TEST(error_desc.actual_type.type == fs::lang::single_object_type::string);

			const range_type input_range(input.begin(), input.end());
			const range_type expected_place_of_expression = search(input_range, "\"33\"");
			const range_type reported_place_of_expression = parse_data.lookup_data.position_of(error_desc.place_of_expression);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input_range));
		}

		BOOST_AUTO_TEST_CASE(empty_socket_group)
		{
			const std::string input_str = fsut::parser::minimal_input() + R"(
const group = Group("")
)";
			const auto input = std::string_view(input_str.data(), input_str.size());
			const fs::parser::parse_data parse_data = parse(input);
			const auto error_desc = expect_error_when_resolving_constants<error::empty_socket_group>(parse_data.ast.constant_definitions);

			const range_type input_range(input.begin(), input.end());
			const range_type expected_place_of_expression = search(input_range, "\"\"");
			const range_type reported_place_of_expression = parse_data.lookup_data.position_of(error_desc.place_of_socket_group_string);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input_range));
		}

		BOOST_AUTO_TEST_CASE(illegal_characters_in_socket_group)
		{
			const std::string input_str = fsut::parser::minimal_input() + R"(
const group = Group("GBAC")
)";
			const auto input = std::string_view(input_str.data(), input_str.size());
			const fs::parser::parse_data parse_data = parse(input);
			const auto error_desc = expect_error_when_resolving_constants<error::illegal_characters_in_socket_group>(parse_data.ast.constant_definitions);

			const range_type input_range(input.begin(), input.end());
			const range_type expected_place_of_expression = search(input_range, "\"GBAC\"");
			const range_type reported_place_of_expression = parse_data.lookup_data.position_of(error_desc.place_of_socket_group_string);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input_range));
		}

		BOOST_AUTO_TEST_CASE(invalid_socket_group)
		{
			const std::string input_str = fsut::parser::minimal_input() + R"(
const group = Group("RRRRRRR") # 7 characters
)";
			const auto input = std::string_view(input_str.data(), input_str.size());
			const fs::parser::parse_data parse_data = parse(input);
			const auto error_desc = expect_error_when_resolving_constants<error::invalid_socket_group>(parse_data.ast.constant_definitions);

			const range_type input_range(input.begin(), input.end());
			const range_type expected_place_of_expression = search(input_range, "\"RRRRRRR\"");
			const range_type reported_place_of_expression = parse_data.lookup_data.position_of(error_desc.place_of_socket_group_string);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input_range));
		}

		BOOST_AUTO_TEST_CASE(invalid_minimap_icon_size)
		{
			const std::string input_str = fsut::parser::minimal_input() + R"(
const icon = MinimapIcon(5, green, circle) # size must be in range [0, 2]
)";
			const auto input = std::string_view(input_str.data(), input_str.size());
			const fs::parser::parse_data parse_data = parse(input);
			const auto error_desc = expect_error_when_resolving_constants<error::invalid_minimap_icon_size>(parse_data.ast.constant_definitions);

			BOOST_TEST(error_desc.requested_size == 5);

			const range_type input_range(input.begin(), input.end());
			const range_type expected_place_of_expression = search(input_range, "5");
			const range_type reported_place_of_expression = parse_data.lookup_data.position_of(error_desc.place_of_size_argument);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input_range));
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
