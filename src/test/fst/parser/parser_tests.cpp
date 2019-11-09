#include <fst/common/print_type.hpp>
#include <fst/common/test_fixtures.hpp>

#include <fs/parser/parser.hpp>
#include <fs/log/buffered_logger.hpp>
#include <fs/utility/holds_alternative.hpp>
#include <fs/version.hpp>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string>

template <typename Value>
bool test_literal(const fs::parser::ast::string_literal& str_lit, const Value& value)
{
	if (str_lit != value)
	{
		BOOST_ERROR("failed comparison of " << str_lit << " and " << value);
		return false;
	}

	return true;
}

template <typename T, typename Value>
bool test_literal(const T& literal, const Value& value)
{
	if (literal.value != value)
	{
		BOOST_ERROR("failed comparison of " << literal.value << " and " << value);
		return false;
	}

	return true;
}

template <typename T, typename Value>
bool test_literal_expression(const fs::parser::ast::value_expression& expr, const Value& value)
{
	BOOST_TEST(expr.postfix_exprs.empty(), "definition should not contain postfix expressions");

	if (!holds_alternative<fs::parser::ast::literal_expression>(expr.primary_expr.var))
	{
		BOOST_ERROR("definition does not hold literal_expression variant");
		return false;
	}

	const auto& literal_expression = boost::get<fs::parser::ast::literal_expression>(expr.primary_expr.var);
	if (!holds_alternative<T>(literal_expression.var))
	{
		BOOST_ERROR("definition is literal_expression but does not hold expected literal");
		return false;
	}

	const auto& literal = boost::get<T>(literal_expression.var);
	return test_literal(literal, value);
}

template <typename T, typename Value>
bool test_literal_definition(const fs::parser::ast::definition& def, const char* name, const Value& value)
{
	BOOST_TEST(def.definition.name.value == name);
	return test_literal_expression<T>(def.definition.value, value);
}

void test_identifier_definition(const fs::parser::ast::definition& def, const char* name, const char* identifier)
{
	BOOST_TEST(def.definition.name.value == name);
	const fs::parser::ast::primary_expression& prim_expr = def.definition.value.primary_expr;

	if (!holds_alternative<fs::parser::ast::identifier>(prim_expr.var))
	{
		BOOST_ERROR("primary expression does not hold identifier");
		return;
	}

	const auto& id = boost::get<fs::parser::ast::identifier>(prim_expr.var);
	BOOST_TEST(id.value == identifier);
}

namespace fst
{

BOOST_FIXTURE_TEST_SUITE(parser_suite, parser_fixture)
BOOST_AUTO_TEST_SUITE(parser_success_suite)

	BOOST_AUTO_TEST_CASE(version_requirement)
	{
		using namespace fs;
		const parser::ast::ast_type ast = parse(minimal_input()).ast;

		const parser::ast::config& config = ast.config;
		BOOST_TEST(config.params.empty());

		const auto version = ast.version_data.get_value();
		BOOST_TEST(version.major == version::major);
		BOOST_TEST(version.minor == version::minor);
		BOOST_TEST(version.patch == version::patch);
	}

	BOOST_AUTO_TEST_CASE(comments)
	{
		const std::string input = minimal_input() + R"(
#
# test that parser correctly handles all comments, including this one
##
# #
n1 = 1 #
n2 = 2 ## #text
)";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input).ast;

		const std::vector<pa::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 2);

		test_literal_definition<pa::integer_literal>(defs[0], "n1", 1);
		test_literal_definition<pa::integer_literal>(defs[1], "n2", 2);
	}

	BOOST_AUTO_TEST_CASE(identifiers)
	{
		const std::string input = minimal_input() + R"(
# test that parser can correctly parse various identifiers
n1 = 1
n_2 = 2
n__3__ = 3
bUt_RaIdEr_Is_fAsTeR = 4
gOtTa_BuIlD_sOmE_dEfEnSe = 5
GGG = 666
not_a_keyword1 = ttrue
not_a_keyword2 = falsee
Identifiedd = Corruptedd
)";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input).ast;

		const std::vector<pa::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 9);

		test_literal_definition<pa::integer_literal>(defs[0], "n1", 1);
		test_literal_definition<pa::integer_literal>(defs[1], "n_2", 2);
		test_literal_definition<pa::integer_literal>(defs[2], "n__3__", 3);
		test_literal_definition<pa::integer_literal>(defs[3], "bUt_RaIdEr_Is_fAsTeR", 4);
		test_literal_definition<pa::integer_literal>(defs[4], "gOtTa_BuIlD_sOmE_dEfEnSe", 5);
		test_literal_definition<pa::integer_literal>(defs[5], "GGG", 666);
		test_identifier_definition(defs[6], "not_a_keyword1", "ttrue");
		test_identifier_definition(defs[7], "not_a_keyword2", "falsee");
		test_identifier_definition(defs[8], "Identifiedd", "Corruptedd");
	}

	BOOST_AUTO_TEST_CASE(empty_string)
	{
		const std::string input = minimal_input() + "\n"
			"empty_string = \"\"";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input).ast;

		const std::vector<pa::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 1);

		test_literal_definition<pa::string_literal>(defs[0], "empty_string", "");
	}

	BOOST_AUTO_TEST_CASE(color_definitions)
	{
		const std::string input = minimal_input() + "\n"
			"color_first = RGB( 11,  22,  33)\n"
			"color_black = RGB(  0,   1,   2, 255)\n"
			"color_other = color_black";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input).ast;

		const std::vector<pa::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 3);

		BOOST_TEST(defs[0].definition.name.value == "color_first");
		BOOST_TEST(defs[1].definition.name.value == "color_black");
		BOOST_TEST(defs[2].definition.name.value == "color_other");

		BOOST_TEST(defs[0].definition.value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::function_call>(defs[0].definition.value.primary_expr.var));
		const auto& f0 = boost::get<pa::function_call>(defs[0].definition.value.primary_expr.var);
		BOOST_TEST(f0.name.value == "RGB");
		BOOST_TEST_REQUIRE(static_cast<int>(f0.arguments.size()) == 3);

		test_literal_expression<pa::integer_literal>(f0.arguments[0], 11);
		test_literal_expression<pa::integer_literal>(f0.arguments[1], 22);
		test_literal_expression<pa::integer_literal>(f0.arguments[2], 33);

		BOOST_TEST(defs[1].definition.value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::function_call>(defs[1].definition.value.primary_expr.var));
		const auto& f1 = boost::get<pa::function_call>(defs[1].definition.value.primary_expr.var);
		BOOST_TEST(f1.name.value == "RGB");
		BOOST_TEST_REQUIRE(static_cast<int>(f1.arguments.size()) == 4);

		test_literal_expression<pa::integer_literal>(f1.arguments[0], 0);
		test_literal_expression<pa::integer_literal>(f1.arguments[1], 1);
		test_literal_expression<pa::integer_literal>(f1.arguments[2], 2);
		test_literal_expression<pa::integer_literal>(f1.arguments[3], 255);

		BOOST_TEST(defs[2].definition.value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(defs[2].definition.value.primary_expr.var));
		const auto& iden = boost::get<pa::identifier>(defs[2].definition.value.primary_expr.var);
		BOOST_TEST(iden.value == "color_black");
	}

	BOOST_AUTO_TEST_CASE(array_definition)
	{
		const std::string input = minimal_input() + "\n"
			"currency_t1 = [\"Exalted Orb\", \"Mirror of Kalandra\", \"Eternal Orb\", \"Mirror Shard\"]";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input).ast;

		const std::vector<pa::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 1);

		BOOST_TEST(defs[0].definition.name.value == "currency_t1");
		BOOST_TEST(defs[0].definition.value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::array_expression>(defs[0].definition.value.primary_expr.var));
		const auto& array_expr = boost::get<pa::array_expression>(defs[0].definition.value.primary_expr.var);
		BOOST_TEST_REQUIRE(static_cast<int>(array_expr.elements.size()) == 4);

		const std::vector<std::string> names = { "Exalted Orb", "Mirror of Kalandra", "Eternal Orb", "Mirror Shard" };
		BOOST_TEST_REQUIRE(array_expr.elements.size() == names.size()); // sanity check

		for (int i = 0; i < static_cast<int>(array_expr.elements.size()); ++i) {
			if (!test_literal_expression<pa::string_literal>(array_expr.elements[i], names[i])) {
				BOOST_ERROR("failed literal expressino for i = " << i);
			}
		}
	}

	BOOST_AUTO_TEST_CASE(nested_block_definition)
	{
		// note: parser does not evaluate identifier references
		// it's done later by the compiler which would error upon
		// this source but for the parser - AST is correct here
		const std::string input = minimal_input() + R"(
# sample comment
SetBackgroundColor color_black

Class "Currency" {
	SetBorderColor color_currency

	BaseType currency_t1 {
		SetAlertSound sound_currency
		Show
	}

	Hide
}

Show
)";
		namespace lang = fs::lang;
		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input).ast;

		const std::vector<pa::statement>& statements = ast.statements;
		BOOST_TEST_REQUIRE(static_cast<int>(statements.size()) == 3);
		/**
		 * Testing anything deeper turned out to be more of a maintenance burden than profit.
		 * Filter generation tests already cover this functionality. The parser's private AST
		 * API is too volatile to fix such tests on every grammar change. Just write different
		 * tests and ensure parsing itself succeeds.
		 */
	}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
