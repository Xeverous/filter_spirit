#include "common/print_type.hpp"
#include "common/test_fixtures.hpp"
#include "common/string_operations.hpp"

#include <fs/compiler/print_error.hpp>
#include <fs/compiler/compiler.hpp>
#include <fs/log/string_logger.hpp>
#include <fs/utility/string_helpers.hpp>

#include <boost/test/unit_test.hpp>

#include <string>
#include <string_view>
#include <functional>
#include <utility>

namespace ut = boost::unit_test;
namespace tt = boost::test_tools;

namespace fs::test
{

using compiler::compile_error;

BOOST_FIXTURE_TEST_SUITE(compiler_suite, compiler_fixture)

	BOOST_AUTO_TEST_CASE(minimal_input_resolve_constants)
	{
		const parser::sf::parse_success_data parse_data = parse(minimal_input());
		const std::variant<lang::symbol_table, compile_error> symbols_or_error =
			resolve_symbols(parse_data.ast.definitions);
		BOOST_TEST_REQUIRE(std::holds_alternative<lang::symbol_table>(symbols_or_error));
		const auto& symbols = std::get<lang::symbol_table>(symbols_or_error);
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
			const compile_error& error,
			const parser::lookup_data& lookup_data)
		{
			if (!std::holds_alternative<T>(error)) {
				log::string_logger logger;
				compiler::print_error(error, lookup_data, logger);
				BOOST_FAIL("got error of a different type than expected:\n" << logger.str());
			}

			return std::get<T>(error);
		}

		compile_error expect_error_when_resolving_symbols(
			const std::vector<parser::ast::sf::definition>& defs)
		{
			std::variant<lang::symbol_table, compile_error> symbols_or_error = resolve_symbols(defs);
			BOOST_TEST_REQUIRE(std::holds_alternative<compile_error>(symbols_or_error));
			return std::get<compile_error>(std::move(symbols_or_error));
		}

		compile_error expect_error_when_compiling(
			const parser::ast::sf::filter_structure& fs)
		{
			std::variant<lang::symbol_table, compile_error> symbols_or_error = resolve_symbols(fs.definitions);
			BOOST_TEST_REQUIRE(std::holds_alternative<lang::symbol_table>(symbols_or_error));
			auto& symbols = std::get<lang::symbol_table>(symbols_or_error);
			std::variant<lang::item_filter, compile_error> filter_or_err = compiler::compile_spirit_filter(fs.statements, symbols);
			BOOST_TEST_REQUIRE(std::holds_alternative<compile_error>(filter_or_err));
			return std::get<compile_error>(filter_or_err);
		}
	};

	BOOST_FIXTURE_TEST_SUITE(compiler_error_suite, compiler_error_fixture,
		* ut::depends_on("compiler_suite/minimal_input_resolve_constants"))

		namespace errors = compiler::errors;

		BOOST_AUTO_TEST_CASE(name_already_exists)
		{
			const std::string input_str = minimal_input() + R"(
some_var = 0
xyz = 1
some_other_var = 2
xyz = 3
)";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::name_already_exists>(error, parse_data.lookup_data);

			const std::string_view pattern = "xyz";
			const std::string_view expected_original_name = search(input, pattern).result();
			const std::string_view reported_original_name = parse_data.lookup_data.position_of(error_desc.original_name);
			BOOST_TEST(compare_ranges(expected_original_name, reported_original_name, input));

			const char *const search_first = expected_original_name.data() + expected_original_name.size();
			const char *const search_last  = input.data() + input.size();
			const std::string_view expected_duplicated_name = search(utility::make_string_view(search_first, search_last), pattern).result();
			const std::string_view reported_duplicated_name = parse_data.lookup_data.position_of(error_desc.duplicated_name);
			BOOST_TEST(compare_ranges(expected_duplicated_name, reported_duplicated_name, input));
		}

		BOOST_AUTO_TEST_CASE(no_such_name)
		{
			const std::string input_str = minimal_input() + R"(
abc = 0
xyz = non_existent_obj
)";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const compile_error error = expect_error_when_resolving_symbols(parse_data.ast.definitions);
			const auto& error_desc = expect_error_of_type<errors::no_such_name>(error, parse_data.lookup_data);

			const std::string_view expected_name = search(input, "non_existent_obj").result();
			const std::string_view reported_name = parse_data.lookup_data.position_of(error_desc.name);
			BOOST_TEST(compare_ranges(expected_name, reported_name, input));
		}

		BOOST_AUTO_TEST_CASE(invalid_amount_of_arguments)
		{
			const std::string input_str = minimal_input() + R"(PlayAlertSound 11 22 33)";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const compile_error error = expect_error_when_compiling(parse_data.ast);
			const auto& error_desc = expect_error_of_type<errors::invalid_amount_of_arguments>(error, parse_data.lookup_data);

			BOOST_TEST(error_desc.min_allowed == 1);
			BOOST_TEST(error_desc.max_allowed == 2);
			BOOST_TEST(error_desc.actual == 3);

			const std::string_view expected_arguments = search(input, "11 22 33").result();
			const std::string_view reported_arguments = parse_data.lookup_data.position_of(error_desc.arguments);
			BOOST_TEST(compare_ranges(expected_arguments, reported_arguments, input));
		}

		BOOST_AUTO_TEST_CASE(invalid_integer_value)
		{
			const std::string input_str = minimal_input() + R"(MinimapIcon 3 Brown Circle)";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const compile_error error = expect_error_when_compiling(parse_data.ast);
			const auto& error_desc = expect_error_of_type<errors::invalid_integer_value>(error, parse_data.lookup_data);

			BOOST_TEST(error_desc.min_allowed_value == 0);
			BOOST_TEST(error_desc.max_allowed_value == 2);
			BOOST_TEST(error_desc.actual_value == 3);

			const std::string_view expected_argument = search(input, "3").result();
			const std::string_view reported_argument = parse_data.lookup_data.position_of(error_desc.origin);
			BOOST_TEST(compare_ranges(expected_argument, reported_argument, input));
		}

		BOOST_AUTO_TEST_CASE(type_mismatch)
		{
			const std::string input_str = minimal_input() + R"(BaseType 123 {})";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const compile_error error = expect_error_when_compiling(parse_data.ast);
			const auto& error_desc = expect_error_of_type<errors::type_mismatch>(error, parse_data.lookup_data);

			BOOST_TEST(error_desc.expected_type == +lang::object_type::string);
			BOOST_TEST(error_desc.actual_type == +lang::object_type::integer);

			const std::string_view expected_expression = search(input, "123").result();
			const std::string_view reported_expression = parse_data.lookup_data.position_of(error_desc.expression);
			BOOST_TEST(compare_ranges(expected_expression, reported_expression, input));
		}

		BOOST_AUTO_TEST_CASE(duplicate_influence)
		{
			const std::string input_str = minimal_input() + R"(HasInfluence Shaper Shaper {})";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const compile_error error = expect_error_when_compiling(parse_data.ast);
			const auto& error_desc = expect_error_of_type<errors::duplicate_influence>(error, parse_data.lookup_data);

			const std::string_view expected_first = search(input, "Shaper").result();
			const std::string_view reported_first = parse_data.lookup_data.position_of(error_desc.first_influence);
			BOOST_TEST(compare_ranges(expected_first, reported_first, input));

			const std::string_view expected_second = search(input, "Shaper").next().result();
			const std::string_view reported_second = parse_data.lookup_data.position_of(error_desc.second_influence);
			BOOST_TEST(compare_ranges(expected_second, reported_second, input));
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
			const parser::sf::parse_success_data parse_data = parse(input);
			const compile_error error = expect_error_when_compiling(parse_data.ast);
			const auto& error_desc = expect_error_of_type<errors::condition_redefinition>(error, parse_data.lookup_data);

			const std::string_view expected_original = search(input, "Class \"Boots\"").result();
			const std::string_view reported_original = parse_data.lookup_data.position_of(error_desc.original_definition);
			BOOST_TEST(compare_ranges(expected_original, reported_original, input));

			const std::string_view expected_redef = search(input, "Class \"Gloves\"").result();
			const std::string_view reported_redef = parse_data.lookup_data.position_of(error_desc.redefinition);
			BOOST_TEST(compare_ranges(expected_redef, reported_redef, input));
		}

		BOOST_AUTO_TEST_CASE(lower_bound_redefinition)
		{
			const std::string input_str = minimal_input() + R"(
Quality = 10
Quality > 0
{}
)";
			const std::string_view input = input_str;
			const parser::sf::parse_success_data parse_data = parse(input);
			const compile_error error = expect_error_when_compiling(parse_data.ast);
			const auto& error_desc = expect_error_of_type<errors::lower_bound_redefinition>(error, parse_data.lookup_data);

			const std::string_view expected_original = search(input, "Quality").result();
			const std::string_view reported_original = parse_data.lookup_data.position_of(error_desc.original_definition);
			BOOST_TEST(compare_ranges(expected_original, reported_original, input));

			const std::string_view expected_redef = search(input, "Quality").next().result();
			const std::string_view reported_redef = parse_data.lookup_data.position_of(error_desc.redefinition);
			BOOST_TEST(compare_ranges(expected_redef, reported_redef, input));
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
