#include "fst/common/print_type.hpp"
#include "fst/common/test_fixtures.hpp"

#include "fs/parser/parser.hpp"
#include "fs/log/buffered_logger.hpp"
#include "fs/core/version.hpp"
#include "fs/utility/holds_alternative.hpp"

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
bool test_literal_definition(const fs::parser::ast::constant_definition& def, const char* name, const Value& value)
{
	BOOST_TEST(def.name.value == name);
	return test_literal_expression<T>(def.value, value);
}

void test_identifier_definition(const fs::parser::ast::constant_definition& def, const char* name, const char* identifier)
{
	BOOST_TEST(def.name.value == name);
	const fs::parser::ast::primary_expression& prim_expr = def.value.primary_expr;

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
		BOOST_TEST(version.major == core::version::major);
		BOOST_TEST(version.minor == core::version::minor);
		BOOST_TEST(version.patch == core::version::patch);
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

		const std::vector<pa::constant_definition>& defs = ast.constant_definitions;
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
Idendified = Corrrupted
)";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input).ast;

		const std::vector<pa::constant_definition>& defs = ast.constant_definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 9);

		test_literal_definition<pa::integer_literal>(defs[0], "n1", 1);
		test_literal_definition<pa::integer_literal>(defs[1], "n_2", 2);
		test_literal_definition<pa::integer_literal>(defs[2], "n__3__", 3);
		test_literal_definition<pa::integer_literal>(defs[3], "bUt_RaIdEr_Is_fAsTeR", 4);
		test_literal_definition<pa::integer_literal>(defs[4], "gOtTa_BuIlD_sOmE_dEfEnSe", 5);
		test_literal_definition<pa::integer_literal>(defs[5], "GGG", 666);
		test_identifier_definition(defs[6], "not_a_keyword1", "ttrue");
		test_identifier_definition(defs[7], "not_a_keyword2", "falsee");
		test_identifier_definition(defs[8], "Idendified", "Corrrupted");
	}

	BOOST_AUTO_TEST_CASE(empty_string)
	{
		const std::string input = minimal_input() + "\n"
			"empty_string = \"\"";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input).ast;

		const std::vector<pa::constant_definition>& defs = ast.constant_definitions;
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

		const std::vector<pa::constant_definition>& defs = ast.constant_definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 3);

		BOOST_TEST(defs[0].name.value == "color_first");
		BOOST_TEST(defs[1].name.value == "color_black");
		BOOST_TEST(defs[2].name.value == "color_other");

		BOOST_TEST(defs[0].value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::function_call>(defs[0].value.primary_expr.var));
		const auto& f0 = boost::get<pa::function_call>(defs[0].value.primary_expr.var);
		BOOST_TEST(f0.name.value == "RGB");
		BOOST_TEST_REQUIRE(static_cast<int>(f0.arguments.size()) == 3);

		test_literal_expression<pa::integer_literal>(f0.arguments[0], 11);
		test_literal_expression<pa::integer_literal>(f0.arguments[1], 22);
		test_literal_expression<pa::integer_literal>(f0.arguments[2], 33);

		BOOST_TEST(defs[1].value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::function_call>(defs[1].value.primary_expr.var));
		const auto& f1 = boost::get<pa::function_call>(defs[1].value.primary_expr.var);
		BOOST_TEST(f1.name.value == "RGB");
		BOOST_TEST_REQUIRE(static_cast<int>(f1.arguments.size()) == 4);

		test_literal_expression<pa::integer_literal>(f1.arguments[0], 0);
		test_literal_expression<pa::integer_literal>(f1.arguments[1], 1);
		test_literal_expression<pa::integer_literal>(f1.arguments[2], 2);
		test_literal_expression<pa::integer_literal>(f1.arguments[3], 255);

		BOOST_TEST(defs[2].value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(defs[2].value.primary_expr.var));
		const auto& iden = boost::get<pa::identifier>(defs[2].value.primary_expr.var);
		BOOST_TEST(iden.value == "color_black");
	}

	BOOST_AUTO_TEST_CASE(array_definition)
	{
		const std::string input = minimal_input() + "\n"
			"currency_t1 = [\"Exalted Orb\", \"Mirror of Kalandra\", \"Eternal Orb\", \"Mirror Shard\"]";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input).ast;

		const std::vector<pa::constant_definition>& defs = ast.constant_definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 1);

		BOOST_TEST(defs[0].name.value == "currency_t1");
		BOOST_TEST(defs[0].value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::array_expression>(defs[0].value.primary_expr.var));
		const auto& array_expr = boost::get<pa::array_expression>(defs[0].value.primary_expr.var);
		BOOST_TEST_REQUIRE(static_cast<int>(array_expr.elements.size()) == 4);

		const std::vector<std::string> names = { "Exalted Orb", "Mirror of Kalandra", "Eternal Orb", "Mirror Shard" };
		BOOST_TEST_REQUIRE(array_expr.elements.size() == names.size()); // sanity check

		for (int i = 0; i < static_cast<int>(array_expr.elements.size()); ++i)
		{
			if (!test_literal_expression<pa::string_literal>(array_expr.elements[i], names[i]))
			{
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

		BOOST_TEST_REQUIRE(holds_alternative<pa::action>(statements[0].var)); // SetBackgroundColor color_black
		const auto& st0_action = boost::get<pa::action>(statements[0].var);
		const auto& st0_unary_action = st0_action.action;
		BOOST_TEST(st0_unary_action.action_type == lang::unary_action_type::set_background_color);
		BOOST_TEST(st0_unary_action.value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(st0_unary_action.value.primary_expr.var));
		const auto& st0_action_id = boost::get<pa::identifier>(st0_unary_action.value.primary_expr.var);
		BOOST_TEST(st0_action_id.value =="color_black");

		BOOST_TEST_REQUIRE(holds_alternative<pa::rule_block>(statements[1].var)); // Class "Currency" {
		const auto& st1_rule_block = boost::get<pa::rule_block>(statements[1].var);
		BOOST_TEST_REQUIRE(static_cast<int>(st1_rule_block.conditions.size()) == 1);
		const auto& st1_rule_block_cond = st1_rule_block.conditions[0];
		BOOST_TEST_REQUIRE(holds_alternative<pa::string_condition>(st1_rule_block_cond.var));
		const auto& st1_rule_block_cond_str = boost::get<pa::string_condition>(st1_rule_block_cond.var);
		BOOST_TEST(st1_rule_block_cond_str.property == lang::string_condition_property::class_);
		BOOST_TEST(st1_rule_block_cond_str.value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(st1_rule_block_cond_str.value.primary_expr.var));
		const auto& st1_rule_block_cond_str_lit = boost::get<pa::literal_expression>(st1_rule_block_cond_str.value.primary_expr.var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::string_literal>(st1_rule_block_cond_str_lit.var));
		const auto& st1_rule_block_cond_str_lit_str = boost::get<pa::string_literal>(st1_rule_block_cond_str_lit.var);
		BOOST_TEST(st1_rule_block_cond_str_lit_str == "Currency");

		BOOST_TEST_REQUIRE(static_cast<int>(st1_rule_block.statements.size()) == 3);

		const auto& st1_st0 = st1_rule_block.statements[0]; // SetBorderColor color_currency
		BOOST_TEST_REQUIRE(holds_alternative<pa::action>(st1_st0.var));
		const auto& st1_st0_action = boost::get<pa::action>(st1_st0.var);
		const auto& st1_st0_unary_action = st1_st0_action.action;
		BOOST_TEST(st1_st0_unary_action.action_type == lang::unary_action_type::set_border_color);
		BOOST_TEST(st1_st0_unary_action.value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(st1_st0_unary_action.value.primary_expr.var));
		const auto& st1_st0_action_id = boost::get<pa::identifier>(st1_st0_unary_action.value.primary_expr.var);
		BOOST_TEST(st1_st0_action_id.value == "color_currency");

		const auto& st1_st1 = st1_rule_block.statements[1]; // BaseType currency_t1
		BOOST_TEST_REQUIRE(holds_alternative<pa::rule_block>(st1_st1.var));
		const auto& st1_st1_cond = boost::get<pa::rule_block>(st1_st1.var);
		BOOST_TEST_REQUIRE(static_cast<int>(st1_st1_cond.conditions.size()) == 1);
		const auto& st1_st1_cond0 = st1_st1_cond.conditions[0];
		BOOST_TEST_REQUIRE(holds_alternative<pa::string_condition>(st1_st1_cond0.var));
		const auto& st1_st1_cond0_str = boost::get<pa::string_condition>(st1_st1_cond0.var);
		BOOST_TEST(st1_st1_cond0_str.property == lang::string_condition_property::base_type);
		BOOST_TEST(st1_st1_cond0_str.value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(st1_st1_cond0_str.value.primary_expr.var));
		const auto& st1_st1_cond0_str_lit = boost::get<pa::identifier>(st1_st1_cond0_str.value.primary_expr.var);
		BOOST_TEST(st1_st1_cond0_str_lit.value == "currency_t1");

		BOOST_TEST_REQUIRE(static_cast<int>(st1_st1_cond.statements.size()) == 2);

		const auto& st1_st1_cond_st0 = st1_st1_cond.statements[0]; // SetAlertSound sound_currency
		BOOST_TEST_REQUIRE(holds_alternative<pa::action>(st1_st1_cond_st0.var));
		const auto& st1_st1_cond_st0_action = boost::get<pa::action>(st1_st1_cond_st0.var);
		const auto& st1_st1_cond_st0_unary_action = st1_st1_cond_st0_action.action;
		BOOST_TEST(st1_st1_cond_st0_unary_action.action_type == lang::unary_action_type::set_alert_sound);
		BOOST_TEST(st1_st1_cond_st0_unary_action.value.postfix_exprs.empty());
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(st1_st1_cond_st0_unary_action.value.primary_expr.var));
		const auto& st1_st1_cond_st0_action_id = boost::get<pa::identifier>(st1_st1_cond_st0_unary_action.value.primary_expr.var);
		BOOST_TEST(st1_st1_cond_st0_action_id.value == "sound_currency");

		const auto& st1_st1_cond_st1 = st1_st1_cond.statements[1]; // Show
		BOOST_TEST_REQUIRE(holds_alternative<pa::visibility_statement>(st1_st1_cond_st1.var));
		const auto& st1_st1_cond_st1_vs = boost::get<pa::visibility_statement>(st1_st1_cond_st1.var);
		BOOST_TEST(st1_st1_cond_st1_vs.show == true);

		const auto& st1_st2 = st1_rule_block.statements[2]; // Hide
		BOOST_TEST_REQUIRE(holds_alternative<pa::visibility_statement>(st1_st2.var));
		const auto& st1_st2_vs = boost::get<pa::visibility_statement>(st1_st2.var);
		BOOST_TEST(st1_st2_vs.show == false);

		const auto& st2 = statements[2];
		BOOST_TEST_REQUIRE(holds_alternative<pa::visibility_statement>(st2.var));
		const auto& st2_vs = boost::get<pa::visibility_statement>(st2.var);
		BOOST_TEST(st2_vs.show == true);
	}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
