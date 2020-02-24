#include "common/print_type.hpp"
#include "common/test_fixtures.hpp"

#include <fs/parser/parser.hpp>
#include <fs/utility/holds_alternative.hpp>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace fs;
namespace sf = parser::ast::sf;

namespace {

template <typename Value>
bool test_literal(const sf::string_literal& str_lit, const Value& value)
{
	if (str_lit != value) {
		BOOST_ERROR("failed comparison of " << str_lit << " and " << value);
		return false;
	}

	return true;
}

// assumes the the literal has "value" member
template <typename T, typename Value>
bool test_literal(const T& literal, const Value& value)
{
	if (literal.value != value) {
		BOOST_ERROR("failed comparison of " << literal.value << " and " << value);
		return false;
	}

	return true;
}

const sf::primitive_value* get_primitive(const sf::definition& def, const char* name)
{
	BOOST_TEST(def.def.value_name.value.value == name);

	if (!holds_alternative<sf::sequence>(def.def.value.var)) {
		BOOST_ERROR("expected sequence");
		return nullptr;
	}

	auto& seq = boost::get<sf::sequence>(def.def.value.var);
	if (seq.size() != 1u) {
		BOOST_ERROR("sequence should have size of 1");
		return nullptr;
	}

	return &seq[0];
}

template <typename Literal, typename Value>
bool test_literal_expression(const sf::primitive_value& prim, const Value& value)
{
	if (!holds_alternative<sf::literal_expression>(prim.var)) {
		BOOST_ERROR("expected literal expression");
		return false;
	}

	const auto& expr = boost::get<sf::literal_expression>(prim.var);
	if (!holds_alternative<Literal>(expr.var)) {
		BOOST_ERROR("definition is literal_expression but does not hold expected literal");
		return false;
	}

	const auto& literal = boost::get<Literal>(expr.var);
	return test_literal(literal, value);
}

template <typename Literal, typename Value>
bool test_literal_definition(const sf::definition& def, const char* name, const Value& value)
{
	const sf::primitive_value* prim_ptr = get_primitive(def, name);

	if (prim_ptr == nullptr) {
		BOOST_ERROR("expected primitive");
		return false;
	}

	return test_literal_expression<Literal>(*prim_ptr, value);
}

bool test_identifier_definition(const sf::definition& def, const char* name, const char* identifier)
{
	const sf::primitive_value* prim_ptr = get_primitive(def, name);

	if (prim_ptr == nullptr) {
		BOOST_ERROR("expected primitive");
		return false;
	}

	const auto& prim = *prim_ptr;
	if (!holds_alternative<sf::name>(prim.var)) {
		BOOST_ERROR("expected name");
		return false;
	}

	const auto& iden = boost::get<sf::name>(prim.var).value;
	if (iden.value != identifier) {
		BOOST_ERROR("identifier was expected to be " << identifier << " but is " << iden.value);
		return false;
	}

	return true;
}

} // namespace

namespace fs::test
{

BOOST_FIXTURE_TEST_SUITE(parser_suite, parser_fixture)
BOOST_AUTO_TEST_SUITE(parser_success_suite)

	BOOST_AUTO_TEST_CASE(comments)
	{
		const std::string input = minimal_input() + R"(
#
# test that parser correctly handles all comments, including this one
##
# #
$n1 = 1 #
$n2 = 2 ## #text
)";

		const sf::ast_type ast = parse(input).ast;

		const std::vector<sf::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 2);

		test_literal_definition<sf::integer_literal>(defs[0], "n1", 1);
		test_literal_definition<sf::integer_literal>(defs[1], "n2", 2);
	}

	BOOST_AUTO_TEST_CASE(identifiers)
	{
		const std::string input = minimal_input() + R"(
# test that parser can correctly parse various identifiers
$n1 = 1
$n_2 = 2
$n__3__ = 3
$bUt_RaIdEr_Is_fAsTeR = 4
$gOtTa_BuIlD_sOmE_dEfEnSe = 5
$GGG = 666
$not_a_keyword1 = $ttrue
$not_a_keyword2 = $falsee
$Identifiedd = $Corruptedd
)";

		const sf::ast_type ast = parse(input).ast;

		const std::vector<sf::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 9);

		test_literal_definition<sf::integer_literal>(defs[0], "n1", 1);
		test_literal_definition<sf::integer_literal>(defs[1], "n_2", 2);
		test_literal_definition<sf::integer_literal>(defs[2], "n__3__", 3);
		test_literal_definition<sf::integer_literal>(defs[3], "bUt_RaIdEr_Is_fAsTeR", 4);
		test_literal_definition<sf::integer_literal>(defs[4], "gOtTa_BuIlD_sOmE_dEfEnSe", 5);
		test_literal_definition<sf::integer_literal>(defs[5], "GGG", 666);
		test_identifier_definition(defs[6], "not_a_keyword1", "ttrue");
		test_identifier_definition(defs[7], "not_a_keyword2", "falsee");
		test_identifier_definition(defs[8], "Identifiedd", "Corruptedd");
	}

	BOOST_AUTO_TEST_CASE(empty_string)
	{
		const std::string input = minimal_input() + "\n"
			"$empty_string = \"\"";

		const sf::ast_type ast = parse(input).ast;

		const std::vector<sf::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 1);

		test_literal_definition<sf::string_literal>(defs[0], "empty_string", "");
	}

	BOOST_AUTO_TEST_CASE(integer_sequence)
	{
		const std::string input = minimal_input() + "\n"
			"$integer    = 1\n"
			"$color_rgb  = 0 1 2\n"
			"$color_rgba = 3 4 5 255";

		const sf::ast_type ast = parse(input).ast;

		const std::vector<sf::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 3);

		BOOST_TEST(defs[0].def.value_name.value.value == "integer");
		BOOST_TEST(defs[1].def.value_name.value.value == "color_rgb");
		BOOST_TEST(defs[2].def.value_name.value.value == "color_rgba");

		BOOST_TEST_REQUIRE(holds_alternative<sf::sequence>(defs[0].def.value.var));
		const auto& seq0 = boost::get<sf::sequence>(defs[0].def.value.var);
		BOOST_TEST_REQUIRE(seq0.size() == 1u);
		test_literal_expression<sf::integer_literal>(seq0[0], 1);

		BOOST_TEST_REQUIRE(holds_alternative<sf::sequence>(defs[1].def.value.var));
		const auto& seq1 = boost::get<sf::sequence>(defs[1].def.value.var);
		BOOST_TEST_REQUIRE(seq1.size() == 3u);
		test_literal_expression<sf::integer_literal>(seq1[0], 0);
		test_literal_expression<sf::integer_literal>(seq1[1], 1);
		test_literal_expression<sf::integer_literal>(seq1[2], 2);

		BOOST_TEST_REQUIRE(holds_alternative<sf::sequence>(defs[2].def.value.var));
		const auto& seq2 = boost::get<sf::sequence>(defs[2].def.value.var);
		BOOST_TEST_REQUIRE(seq2.size() == 4u);
		test_literal_expression<sf::integer_literal>(seq2[0], 3);
		test_literal_expression<sf::integer_literal>(seq2[1], 4);
		test_literal_expression<sf::integer_literal>(seq2[2], 5);
		test_literal_expression<sf::integer_literal>(seq2[3], 255);
	}

	BOOST_AUTO_TEST_CASE(string_sequence)
	{
		const std::string input = minimal_input() + "\n"
			"$currency_t1 = \"Exalted Orb\" \"Mirror of Kalandra\" \"Eternal Orb\" \"Mirror Shard\"";

		const sf::ast_type ast = parse(input).ast;

		const std::vector<sf::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(static_cast<int>(defs.size()) == 1);

		BOOST_TEST(defs[0].def.value_name.value.value == "currency_t1");

		auto& val_expr = defs[0].def.value;
		BOOST_TEST_REQUIRE(holds_alternative<sf::sequence>(val_expr.var));

		auto& seq = boost::get<sf::sequence>(val_expr.var);
		BOOST_TEST(seq.size() == 4u);

		const std::vector<std::string> names = { "Exalted Orb", "Mirror of Kalandra", "Eternal Orb", "Mirror Shard" };
		BOOST_TEST_REQUIRE(seq.size() == names.size()); // sanity check

		for (std::size_t i = 0; i < seq.size(); ++i) {
			if (!test_literal_expression<sf::string_literal>(seq[i], names[i])) {
				BOOST_ERROR("failed literal expression for i = " << i);
			}
		}
	}

	BOOST_AUTO_TEST_CASE(empty_compound_action_definition)
	{
		const std::string input = minimal_input() + "\n"
			"$empty_action = {}";

		const sf::ast_type ast = parse(input).ast;

		const std::vector<sf::definition>& defs = ast.definitions;
		BOOST_TEST_REQUIRE(defs.size() == 1u);

		BOOST_TEST(defs[0].def.value_name.value.value == "empty_action");

		auto& val_expr = defs[0].def.value;
		BOOST_TEST_REQUIRE(holds_alternative<sf::compound_action_expression>(val_expr.var));

		auto& ca_expr = boost::get<sf::compound_action_expression>(val_expr.var);
		BOOST_TEST(ca_expr.empty());
	}

	BOOST_AUTO_TEST_CASE(nested_block_definition)
	{
		// note: parser does not evaluate identifier references
		// it's done later by the compiler which would error upon
		// this source but for the parser - AST is correct here
		const std::string input = minimal_input() + R"(
# sample comment
SetBackgroundColor $color_black

Class "Currency" {
	SetBorderColor $color_currency

	BaseType $currency_t1 {
		PlayAlertSound $sound_currency
		Show
	}

	Hide
}

Show
)";

		const sf::ast_type ast = parse(input).ast;

		const std::vector<sf::statement>& statements = ast.statements;
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
