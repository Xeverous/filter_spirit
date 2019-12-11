#include <fst/common/print_type.hpp>
#include <fst/common/test_fixtures.hpp>
#include <fst/common/string_operations.hpp>

#include <fs/compiler/print_error.hpp>
#include <fs/compiler/resolve_symbols.hpp>
#include <fs/log/buffered_logger.hpp>
#include <fs/log/utility.hpp>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string>
#include <string_view>
#include <functional>
#include <utility>

namespace ut = boost::unit_test;
namespace tt = boost::test_tools;

namespace fst
{

BOOST_FIXTURE_TEST_SUITE(compiler_suite, compiler_fixture)

	BOOST_AUTO_TEST_CASE(minimal_input_resolve_constants)
	{
		const fs::parser::parse_success_data parse_data = parse(minimal_input());
		const std::variant<fs::lang::symbol_table, fs::compiler::compile_error> symbols_or_error =
			resolve_symbols(parse_data.ast.definitions);
		BOOST_TEST_REQUIRE(std::holds_alternative<fs::lang::symbol_table>(symbols_or_error));
		const auto& symbols = std::get<fs::lang::symbol_table>(symbols_or_error);
		BOOST_TEST(symbols.empty());
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
		template <typename T>
		const T& expect_error_of_type(
			const fs::compiler::compile_error& error,
			const fs::parser::lookup_data& lookup_data)
		{
			if (!std::holds_alternative<T>(error))
			{
				fs::log::buffered_logger logger;
				fs::compiler::print_error(error, lookup_data, logger);
				const auto log_data = logger.flush_out();
				BOOST_FAIL("got error of a different type than expected:\n" << log_data);
			}

			return std::get<T>(error);
		}

		fs::compiler::compile_error expect_error_when_resolving_symbols(
			const std::vector<fs::parser::ast::definition>& defs)
		{
			std::variant<fs::lang::symbol_table, fs::compiler::compile_error> symbols_or_error =
				resolve_symbols(defs);
			BOOST_TEST_REQUIRE(std::holds_alternative<fs::compiler::compile_error>(symbols_or_error));
			return std::get<fs::compiler::compile_error>(std::move(symbols_or_error));
		}

		// checks all fields except nested errors
		void test_error(
			const fs::compiler::errors::no_matching_constructor_found& error,
			const fs::parser::lookup_data& lookup_data,
			fs::lang::object_type expected_object_type,
			const std::vector<std::optional<fs::lang::object_type>>& expected_supplied_types,
			std::string_view expected_place_of_function_call)
		{
			BOOST_TEST(expected_object_type == error.attempted_type_to_construct);

			const std::string_view whole_input = lookup_data.get_view_of_whole_content();
			const std::string_view reported_place_of_function_call = lookup_data.position_of(error.place_of_function_call);
			BOOST_TEST(compare_ranges(expected_place_of_function_call, reported_place_of_function_call, whole_input));

			BOOST_TEST(error.supplied_types == expected_supplied_types, tt::per_element());
		}

		// checks fields except nested error
		void test_error(
			const fs::compiler::errors::failed_constructor_call& error,
			const fs::parser::lookup_data& lookup_data,
			fs::lang::object_type expected_attempted_object_type,
			const std::vector<fs::lang::object_type>& expected_types,
			std::string_view expected_place_of_function_call)
		{
			BOOST_TEST(expected_attempted_object_type == error.attempted_type_to_construct);

			const std::string_view whole_input = lookup_data.get_view_of_whole_content();
			const std::string_view reported_place_of_function_call = lookup_data.position_of(error.place_of_function_call);
			BOOST_TEST(compare_ranges(expected_place_of_function_call, reported_place_of_function_call, whole_input));

			BOOST_TEST(error.ctor_argument_types == expected_types, tt::per_element());
		}
	};

	BOOST_FIXTURE_TEST_SUITE(compiler_error_suite, compiler_error_fixture, * ut::depends_on("compiler_suite/minimal_input_resolve_constants"))
		namespace errors = fs::compiler::errors;

		BOOST_AUTO_TEST_CASE(name_already_exists)
		{
			const std::string input_str = minimal_input() + R"(
some_var = 0
xyz = 1
some_other_var = 2
xyz = 3
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::name_already_exists>(error, parse_data.lookup_data);

			const std::string_view pattern = "xyz";
			const std::string_view expected_place_of_original_name = search(input, pattern);
			const std::string_view reported_place_of_original_name = parse_data.lookup_data.position_of(error_desc.place_of_original_name);
			BOOST_TEST(compare_ranges(expected_place_of_original_name, reported_place_of_original_name, input));

			const char *const search_first = expected_place_of_original_name.data() + expected_place_of_original_name.size();
			const char *const search_last  = input.data() + input.size();
			const std::string_view expected_place_of_duplicated_name = search(fs::log::make_string_view(search_first, search_last), pattern);
			const std::string_view reported_place_of_duplicated_name = parse_data.lookup_data.position_of(error_desc.place_of_duplicated_name);
			BOOST_TEST(compare_ranges(expected_place_of_duplicated_name, reported_place_of_duplicated_name, input));
		}

		BOOST_AUTO_TEST_CASE(no_such_name)
		{
			const std::string input_str = minimal_input() + R"(
abc = 0
xyz = non_existent_obj
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::no_such_name>(error, parse_data.lookup_data);

			const std::string_view expected_place_of_name = search(input, "non_existent_obj");
			const std::string_view reported_place_of_name = parse_data.lookup_data.position_of(error_desc.place_of_name);
			BOOST_TEST(compare_ranges(expected_place_of_name, reported_place_of_name, input));
		}

		BOOST_AUTO_TEST_CASE(no_such_function)
		{
			const std::string input_str = minimal_input() + R"(
abc = non_existent_func(0)
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::no_such_function>(error, parse_data.lookup_data);

			const std::string_view expected_place_of_name = search(input, "non_existent_func");
			const std::string_view reported_place_of_name = parse_data.lookup_data.position_of(error_desc.place_of_name);
			BOOST_TEST(compare_ranges(expected_place_of_name, reported_place_of_name, input));
		}

		BOOST_AUTO_TEST_CASE(index_out_of_range)
		{
			const std::string input_str = minimal_input() + R"(
x = 5
arr = [1, 2, 3]
elem = arr[x]
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::index_out_of_range>(error, parse_data.lookup_data);

			BOOST_TEST(error_desc.array_size == 3);
			BOOST_TEST(error_desc.requested_index == 5);

			const std::string_view expected_place_of_index_error = search(input, "[x]");
			const std::string_view reported_place_of_index_error = parse_data.lookup_data.position_of(error_desc.place_of_subscript);
			BOOST_TEST(compare_ranges(expected_place_of_index_error, reported_place_of_index_error, input));
		}

		BOOST_AUTO_TEST_CASE(invalid_amount_of_arguments)
		{
			const std::string input_str = minimal_input() + R"(
path = Path(11, 22)
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::failed_constructor_call>(error, parse_data.lookup_data);

			const std::string_view expected_place_of_function_call = search(input, "Path(11, 22)");
			test_error(
				error_desc,
				parse_data.lookup_data,
				fs::lang::object_type::path,
				{fs::lang::object_type::string},
				expected_place_of_function_call);

			BOOST_TEST_REQUIRE((error_desc.error != nullptr));
			const auto& inner_error = expect_error_of_type<errors::invalid_amount_of_arguments>(*error_desc.error, parse_data.lookup_data);
			BOOST_TEST(inner_error.expected == 1);
			BOOST_TEST(inner_error.actual == 2);

			const std::string_view expected_place_of_arguments = search(input, "11, 22");
			const std::string_view reported_place_of_arguments = parse_data.lookup_data.position_of(inner_error.place_of_arguments);
			BOOST_TEST(compare_ranges(expected_place_of_arguments, reported_place_of_arguments, input));
		}

		BOOST_AUTO_TEST_CASE(type_mismatch)
		{
			const std::string input_str = minimal_input() + R"(
path = Path(123)
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::failed_constructor_call>(error, parse_data.lookup_data);

			const std::string_view expected_place_of_function_call = search(input, "Path(123)");
			test_error(
				error_desc,
				parse_data.lookup_data,
				fs::lang::object_type::path,
				{fs::lang::object_type::string},
				expected_place_of_function_call);

			BOOST_TEST_REQUIRE((error_desc.error != nullptr));
			const auto& inner_error = expect_error_of_type<errors::type_mismatch>(*error_desc.error, parse_data.lookup_data);
			BOOST_TEST(inner_error.expected_type == +fs::lang::object_type::string);
			BOOST_TEST(inner_error.actual_type == +fs::lang::object_type::integer);

			const std::string_view expected_place_of_expression = search(input, "123");
			const std::string_view reported_place_of_expression = parse_data.lookup_data.position_of(inner_error.place_of_expression);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input));
		}

		BOOST_AUTO_TEST_CASE(empty_socket_group)
		{
			const std::string input_str = minimal_input() + R"(
group = Group("")
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::failed_constructor_call>(error, parse_data.lookup_data);

			const std::string_view expected_place_of_function_call = search(input, "Group(\"\")");
			test_error(
				error_desc,
				parse_data.lookup_data,
				fs::lang::object_type::socket_group,
				{fs::lang::object_type::string},
				expected_place_of_function_call);

			BOOST_TEST_REQUIRE((error_desc.error != nullptr));
			const auto& inner_error = expect_error_of_type<errors::empty_socket_group>(*error_desc.error, parse_data.lookup_data);

			const std::string_view expected_place_of_expression = search(input, "\"\"");
			const std::string_view reported_place_of_expression = parse_data.lookup_data.position_of(inner_error.place_of_socket_group_string);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input));
		}

		BOOST_AUTO_TEST_CASE(illegal_characters_in_socket_group)
		{
			const std::string input_str = minimal_input() + R"(
group = Group("GBAC")
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::failed_constructor_call>(error, parse_data.lookup_data);

			const std::string_view expected_place_of_function_call = search(input, "Group(\"GBAC\")");
			test_error(
				error_desc,
				parse_data.lookup_data,
				fs::lang::object_type::socket_group,
				{fs::lang::object_type::string},
				expected_place_of_function_call);

			BOOST_TEST_REQUIRE((error_desc.error != nullptr));
			const auto& inner_error = expect_error_of_type<errors::illegal_characters_in_socket_group>(*error_desc.error, parse_data.lookup_data);

			const std::string_view expected_place_of_expression = search(input, "\"GBAC\"");
			const std::string_view reported_place_of_expression = parse_data.lookup_data.position_of(inner_error.place_of_socket_group_string);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input));
		}

		BOOST_AUTO_TEST_CASE(invalid_socket_group)
		{
			const std::string input_str = minimal_input() + R"(
group = Group("RRRRRRR") # 7 characters
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::failed_constructor_call>(error, parse_data.lookup_data);

			const std::string_view expected_place_of_function_call = search(input, "Group(\"RRRRRRR\")");
			test_error(
				error_desc,
				parse_data.lookup_data,
				fs::lang::object_type::socket_group,
				{fs::lang::object_type::string},
				expected_place_of_function_call);

			BOOST_TEST_REQUIRE((error_desc.error != nullptr));
			const auto& inner_error = expect_error_of_type<errors::invalid_socket_group>(*error_desc.error, parse_data.lookup_data);

			const std::string_view expected_place_of_expression = search(input, "\"RRRRRRR\"");
			const std::string_view reported_place_of_expression = parse_data.lookup_data.position_of(inner_error.place_of_socket_group_string);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input));
		}

		BOOST_AUTO_TEST_CASE(invalid_minimap_icon_size)
		{
			const std::string input_str = minimal_input() + R"(
icon = MinimapIcon(1234, Green, Circle) # size must be in range [0, 2]
)";
			const std::string_view input = input_str;
			const fs::parser::parse_success_data parse_data = parse(input);
			const fs::compiler::compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::failed_constructor_call>(error, parse_data.lookup_data);

			const std::string_view expected_place_of_function_call = search(input, "MinimapIcon(1234, Green, Circle)");
			test_error(
				error_desc,
				parse_data.lookup_data,
				fs::lang::object_type::minimap_icon,
				{fs::lang::object_type::integer, fs::lang::object_type::suit, fs::lang::object_type::shape},
				expected_place_of_function_call);

			BOOST_TEST_REQUIRE((error_desc.error != nullptr));
			const auto& inner_error = expect_error_of_type<errors::invalid_minimap_icon_size>(*error_desc.error, parse_data.lookup_data);

			BOOST_TEST(inner_error.requested_size == 1234);

			const std::string_view expected_place_of_expression = search(input, "1234");
			const std::string_view reported_place_of_expression = parse_data.lookup_data.position_of(inner_error.place_of_size_argument);
			BOOST_TEST(compare_ranges(expected_place_of_expression, reported_place_of_expression, input));
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
