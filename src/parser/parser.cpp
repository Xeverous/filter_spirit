#include "ast_adapted.hpp"
#include "parser.hpp"
#include "grammar.hpp"
#include "ast_printer.hpp"
#include <iostream>
#include <functional>

namespace fs::parser
{

ast::ast_type parse(const std::string& source, position_cache& positions, bool& result)
{
	iterator_type it = source.begin();
	const iterator_type end = source.end();
	fs::ast::ast_type ast;

	const auto parser = x3::with<position_cache_tag>(std::ref(positions))[fs::grammar()];

	result = x3::phrase_parse(
		it,
		end,
		parser,
		fs::skipper(),
		ast
	);

	if (it != end or !result) // fail if we did not get a full match
	{
		std::cout << "parse failure at pos: " << it - source.begin() << "\n";
		result = false;
	}

	return ast;
}

void print_ast(const ast::ast_type& ast)
{
	std::cout << boost::fusion::tuple_open('[');
	std::cout << boost::fusion::tuple_close(']');
	std::cout << boost::fusion::tuple_delimiter(", ");

	// no match for 'operator<<' (operand types are 'std::ostream' {aka 'std::basic_ostream<char>'} and 'const fs::ast::code_line')
	//for (const auto& elem : ast)
	//	std::cout << elem.value << "\n";

	fs::ast::identifier id;
	id.value = "abc";
	fs::ast::boolean b;
	b.value = true;
	fs::ast::constant_boolean_definition data;
	data.name = id;
	data.value = b;
	fs::ast::constant_definition d2(data);

	fs::ast::identifier id2;
	id2.value = "abc";
	fs::ast::integer i;
	i.value = 42;
	fs::ast::constant_number_definition num;
	num.name = id2;
	num.value = i;
	fs::ast::constant_definition d3(num);

	std::vector<fs::ast::constant_definition> v = { d2, d3 };

	ast::printer()(ast);
	//std::cout << v;
	//boost::fusion::for_each(v, [] (const auto& x) { std::cout << x << ", ";});
}

}
