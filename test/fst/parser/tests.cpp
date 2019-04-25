#include "fst/lang/print_type.hpp"
#include "fst/parser/common.hpp"

#include "fs/parser/parser.hpp"
#include "fs/log/buffered_logger.hpp"
#include "fs/core/version.hpp"
#include "fs/utility/holds_alternative.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string>

BOOST_FIXTURE_TEST_SUITE(parser_suite, fsut::parser::parser_fixture)

	BOOST_AUTO_TEST_CASE(version_requirement)
	{
		using namespace fs;
		const parser::ast::ast_type ast = parse(fsut::parser::minimal_input());

		const parser::ast::config& config = ast.config;
		BOOST_TEST(config.params.empty());

		const auto version = ast.version_data.get_value();
		BOOST_TEST(version.major == core::version::major);
		BOOST_TEST(version.minor == core::version::minor);
		BOOST_TEST(version.patch == core::version::patch);
	}

	BOOST_AUTO_TEST_CASE(empty_string)
	{
		const std::string input = fsut::parser::minimal_input() + "\n"
			"const empty_string = \"\"";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input);

		const std::vector<pa::constant_definition>& defs = ast.constant_definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 1);

		const auto& constant = defs[0];
		BOOST_TEST(constant.name.value == "empty_string");
		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(constant.value.var));
		const auto& literal_expression = boost::get<pa::literal_expression>(constant.value.var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::string_literal>(literal_expression.var));
		const auto& string_literal = boost::get<pa::string_literal>(literal_expression.var);
		BOOST_TEST(string_literal.empty());
	}

	BOOST_AUTO_TEST_CASE(color_definitions)
	{
		const std::string input = fsut::parser::minimal_input() + "\n"
			"const color_first = RGB( 11,  22,  33)\n"
			"const color_black = RGB(  0,   1,   2, 255)\n"
			"const color_other = color_black";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input);

		const std::vector<pa::constant_definition>& defs = ast.constant_definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 3);

		BOOST_TEST(defs[0].name.value == "color_first");
		BOOST_TEST(defs[1].name.value == "color_black");
		BOOST_TEST(defs[2].name.value == "color_other");

		BOOST_TEST_REQUIRE(holds_alternative<pa::function_call>(defs[0].value.var));
		const auto& f0 = boost::get<pa::function_call>(defs[0].value.var);
		BOOST_TEST(f0.name.value == "RGB");
		BOOST_TEST_REQUIRE(static_cast<int>(f0.arguments.size()) == 3);

		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(f0.arguments[0].var));
		const auto& f0_arg0 = boost::get<pa::literal_expression>(f0.arguments[0].var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::integer_literal>(f0_arg0.var));
		const auto& f0_arg0_int = boost::get<pa::integer_literal>(f0_arg0.var);
		BOOST_TEST(f0_arg0_int.value == 11);

		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(f0.arguments[1].var));
		const auto& f0_arg1 = boost::get<pa::literal_expression>(f0.arguments[1].var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::integer_literal>(f0_arg1.var));
		const auto& f0_arg1_int = boost::get<pa::integer_literal>(f0_arg1.var);
		BOOST_TEST(f0_arg1_int.value == 22);

		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(f0.arguments[2].var));
		const auto& f0_arg2 = boost::get<pa::literal_expression>(f0.arguments[2].var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::integer_literal>(f0_arg2.var));
		const auto& f0_arg2_int = boost::get<pa::integer_literal>(f0_arg2.var);
		BOOST_TEST(f0_arg2_int.value == 33);

		BOOST_TEST_REQUIRE(holds_alternative<pa::function_call>(defs[1].value.var));
		const auto& f1 = boost::get<pa::function_call>(defs[1].value.var);
		BOOST_TEST(f1.name.value == "RGB");
		BOOST_TEST_REQUIRE(static_cast<int>(f1.arguments.size()) == 4);

		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(f1.arguments[0].var));
		const auto& f1_arg0 = boost::get<pa::literal_expression>(f1.arguments[0].var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::integer_literal>(f1_arg0.var));
		const auto& f1_arg0_int = boost::get<pa::integer_literal>(f1_arg0.var);
		BOOST_TEST(f1_arg0_int.value == 0);

		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(f1.arguments[1].var));
		const auto& f1_arg1 = boost::get<pa::literal_expression>(f1.arguments[1].var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::integer_literal>(f1_arg1.var));
		const auto& f1_arg1_int = boost::get<pa::integer_literal>(f1_arg1.var);
		BOOST_TEST(f1_arg1_int.value == 1);

		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(f1.arguments[2].var));
		const auto& f1_arg2 = boost::get<pa::literal_expression>(f1.arguments[2].var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::integer_literal>(f1_arg2.var));
		const auto& f1_arg2_int = boost::get<pa::integer_literal>(f1_arg2.var);
		BOOST_TEST(f1_arg2_int.value == 2);

		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(f1.arguments[3].var));
		const auto& f1_arg3 = boost::get<pa::literal_expression>(f1.arguments[3].var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::integer_literal>(f1_arg3.var));
		const auto& f1_arg3_int = boost::get<pa::integer_literal>(f1_arg3.var);
		BOOST_TEST(f1_arg3_int.value == 255);

		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(defs[2].value.var));
		const auto& iden = boost::get<pa::identifier>(defs[2].value.var);
		BOOST_TEST(iden.value == "color_black");
	}

	BOOST_AUTO_TEST_CASE(array_definition)
	{
		const std::string input = fsut::parser::minimal_input() + "\n"
			"const currency_t1 = [\"Exalted Orb\", \"Mirror of Kalandra\", \"Eternal Orb\", \"Mirror Shard\"]";

		namespace pa = fs::parser::ast;
		const pa::ast_type ast = parse(input);

		const std::vector<pa::constant_definition>& defs = ast.constant_definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 1);

		BOOST_TEST(defs[0].name.value == "currency_t1");
		BOOST_TEST_REQUIRE(holds_alternative<pa::array_expression>(defs[0].value.var));
		const auto& array_expr = boost::get<pa::array_expression>(defs[0].value.var);
		BOOST_TEST_REQUIRE(static_cast<int>(array_expr.elements.size()) == 4);

		const std::vector<std::string> names = { "Exalted Orb", "Mirror of Kalandra", "Eternal Orb", "Mirror Shard" };
		BOOST_TEST_REQUIRE(array_expr.elements.size() == names.size()); // sanity check

		for (int i = 0; i < static_cast<int>(array_expr.elements.size()); ++i)
		{
			const pa::value_expression& elem = array_expr.elements[i];
			BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(elem.var), "i = " << i);
			const auto& lit = boost::get<pa::literal_expression>(elem.var);
			BOOST_TEST_REQUIRE(holds_alternative<pa::string_literal>(lit.var), "i = " << i);
			const auto& str = boost::get<pa::string_literal>(lit.var);
			BOOST_TEST(str == names[i], "i = " << i);
		}
	}

	BOOST_AUTO_TEST_CASE(nested_block_definition)
	{
		// note: parser does not evaluate identifier references
		// it's done later by the compiler which would error upon
		// this source but for the parser - AST is correct here
		const std::string input = fsut::parser::minimal_input() + R"(
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
		const pa::ast_type ast = parse(input);

		const std::vector<pa::statement>& statements = ast.statements;
		BOOST_TEST_REQUIRE(static_cast<int>(statements.size()) == 3);

		BOOST_TEST_REQUIRE(holds_alternative<pa::action>(statements[0].var)); // SetBackgroundColor color_black
		const auto& st0_action = boost::get<pa::action>(statements[0].var);
		BOOST_TEST(st0_action.action_type == lang::action_type::set_background_color);
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(st0_action.value.var));
		const auto& st0_action_id = boost::get<pa::identifier>(st0_action.value.var);
		BOOST_TEST(st0_action_id.value =="color_black");

		BOOST_TEST_REQUIRE(holds_alternative<pa::rule_block>(statements[1].var)); // Class "Currency" {
		const auto& st1_rule_block = boost::get<pa::rule_block>(statements[1].var);
		BOOST_TEST_REQUIRE(static_cast<int>(st1_rule_block.conditions.size()) == 1);
		const auto& st1_rule_block_cond = st1_rule_block.conditions[0];
		BOOST_TEST_REQUIRE(holds_alternative<pa::string_condition>(st1_rule_block_cond.var));
		const auto& st1_rule_block_cond_str = boost::get<pa::string_condition>(st1_rule_block_cond.var);
		BOOST_TEST(st1_rule_block_cond_str.property == lang::string_condition_property::class_);
		BOOST_TEST_REQUIRE(holds_alternative<pa::literal_expression>(st1_rule_block_cond_str.value.var));
		const auto& st1_rule_block_cond_str_lit = boost::get<pa::literal_expression>(st1_rule_block_cond_str.value.var);
		BOOST_TEST_REQUIRE(holds_alternative<pa::string_literal>(st1_rule_block_cond_str_lit.var));
		const auto& st1_rule_block_cond_str_lit_str = boost::get<pa::string_literal>(st1_rule_block_cond_str_lit.var);
		BOOST_TEST(st1_rule_block_cond_str_lit_str == "Currency");

		BOOST_TEST_REQUIRE(static_cast<int>(st1_rule_block.statements.size()) == 3);

		const auto& st1_st0 = st1_rule_block.statements[0]; // SetBorderColor color_currency
		BOOST_TEST_REQUIRE(holds_alternative<pa::action>(st1_st0.var));
		const auto& st1_st0_action = boost::get<pa::action>(st1_st0.var);
		BOOST_TEST(st1_st0_action.action_type == lang::action_type::set_border_color);
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(st1_st0_action.value.var));
		const auto& st1_st0_action_id = boost::get<pa::identifier>(st1_st0_action.value.var);
		BOOST_TEST(st1_st0_action_id.value == "color_currency");

		const auto& st1_st1 = st1_rule_block.statements[1]; // BaseType currency_t1
		BOOST_TEST_REQUIRE(holds_alternative<pa::rule_block>(st1_st1.var));
		const auto& st1_st1_cond = boost::get<pa::rule_block>(st1_st1.var);
		BOOST_TEST_REQUIRE(static_cast<int>(st1_st1_cond.conditions.size()) == 1);
		const auto& st1_st1_cond0 = st1_st1_cond.conditions[0];
		BOOST_TEST_REQUIRE(holds_alternative<pa::string_condition>(st1_st1_cond0.var));
		const auto& st1_st1_cond0_str = boost::get<pa::string_condition>(st1_st1_cond0.var);
		BOOST_TEST(st1_st1_cond0_str.property == lang::string_condition_property::base_type);
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(st1_st1_cond0_str.value.var));
		const auto& st1_st1_cond0_str_lit = boost::get<pa::identifier>(st1_st1_cond0_str.value.var);
		BOOST_TEST(st1_st1_cond0_str_lit.value == "currency_t1");

		BOOST_TEST_REQUIRE(static_cast<int>(st1_st1_cond.statements.size()) == 2);

		const auto& st1_st1_cond_st0 = st1_st1_cond.statements[0]; // SetAlertSound sound_currency
		BOOST_TEST_REQUIRE(holds_alternative<pa::action>(st1_st1_cond_st0.var));
		const auto& st1_st1_cond_st0_action = boost::get<pa::action>(st1_st1_cond_st0.var);
		BOOST_TEST(st1_st1_cond_st0_action.action_type == lang::action_type::set_alert_sound);
		BOOST_TEST_REQUIRE(holds_alternative<pa::identifier>(st1_st1_cond_st0_action.value.var));
		const auto& st1_st1_cond_st0_action_id = boost::get<pa::identifier>(st1_st1_cond_st0_action.value.var);
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
