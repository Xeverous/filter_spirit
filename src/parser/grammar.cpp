/**
 * @file grammar instantiations
 *
 * @details This file is intended for compilation of grammar objects.
 * Use BOOST_SPIRIT_INSTANTIATE here.
 */

#include "ast_adapted.hpp"
#include "grammar_def.hpp"

namespace fs::parser
{

BOOST_SPIRIT_INSTANTIATE(grammar_type, iterator_type, context_type)

/*
 * If you are getting error: 'undefined reference to bool fs::parser::parse_rule[...]'
 * this means that the instantiated parser object does not match x3::phrase_parse().
 *
 * Uncomment lines below to produce compiler (not linker) errors that will
 * list generated types. Then compare them with linker error - you should be able
 * to see which parts of the nested grammar type do not match.
 *
 * Also note that x3::phrase_parse() MUST get some object which will be filled with
 * AST data as the last argument. Calling overload which does not fill any AST will
 * not build.
 */
// int x = grammar_type{};
// int y = iterator_type{};
// int z = context_type{};

}
