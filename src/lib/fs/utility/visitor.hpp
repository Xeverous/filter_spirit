#pragma once

namespace fs::utility
{

/*
 * Note:
 * Boost requires result_type type alias to be present in the visitor type
 * (implementation uses typename Visitor::result_type * instead of decltype(auto)).
 * This can not be deduced from provided lambdas and have to be specified manually.
 *
 * Hence, this visitor will not work for boost variants. To visit boost variants,
 * #include <boost/spirit/home/x3/support/utility/lambda_visitor.hpp> and
 * use boost::spirit::x3::make_lambda_visitor<result_type>(lambdas...) factory.
 */
template <typename... Ts>
struct visitor : Ts...
{
	using Ts::operator()...;
};

template <typename... Ts>
visitor(Ts...) -> visitor<Ts...>;

}
