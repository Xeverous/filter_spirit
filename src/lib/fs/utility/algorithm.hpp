#include <utility>
#include <iterator>
#include <algorithm>
#include <tuple>

namespace fs::utility
{

/**
 * @brief algorithm that moves out unique elements
 *
 * @param first beginning of input range
 * @param last end of input range
 * @param dest_first beginning of output range
 * @param p BinaryPredicate used to compare elements (represents operator==)
 * @return pair of: new end of input range, new end of output range
 *
 * @details
 * Preconditions:
 * - only adjacent elements may compare equal, otherwise the algorithm invariant
 * is broken because it assumes that elements for which predicate returns true are
 * always grouped together (no UB but the result is meaningless)
 * - destination range must not overlap with input range,
 * otherwise the behaviour is undefined
 *
 * Complexity: linear with the size of input range, at most std::distance(first, last)
 * applications of the predicate
 *
 * Exception safety: none: supplied predicate and move constructor of iterated
 * elements must never throw
 *
 * Example:
 * input range: 1 2 2 2 3 3 4 5 5 6
 * output range: (empty)
 *             ||
 *             ||
 *            \||/
 *             \/
 * input range: 2 2 2 3 3 5 5 X X X
 * output range: 1 4 6
 *
 * Notes:
 * to complete the erase-remove idiom (remove elements X which have indeterminate values),
 * after calling this algorithm call input.erase(return_val.first, input.end())
 */
template <typename ForwardIt, typename OutputIt, typename BinaryPredicate>
std::pair<ForwardIt, OutputIt> remove_unique(ForwardIt first, ForwardIt last, OutputIt dest_first, BinaryPredicate p)
{
	// the place where we move unique elements: dest_first
	// the place where we move repeated elements:
	ForwardIt new_input_end = first;

	// pick first instance of (potentially repeated) element
	ForwardIt first_instance = first;

	while (first != last)
	{
		// find first instance of a different element
		while (++first != last && p(*first_instance, *first)) {}

		ForwardIt temp = first_instance;
		if (++first_instance == first) // there is only 1 such element
		{
			/*
			 * move this element to destination range
			 *
			 * we do not test against self-move-assignment because
			 * we assume that input and destination ranges do not overlap
			 *
			 * also, such test would not even be possible because:
			 * - ForwardIt and OutputIt can be different types so dest_first != temp would not compile
			 * - OutputIterators do not have to support equality
			 * - we can not rely on &*dest_first != &*temp because OutputIterators can be iterator adaptors
			 * which return proxy objects in their operator overloads - the only valid use of OutputIterators
			 * are expressions *it = val and ++it in single-pass algorithms
			 */
			*dest_first = std::move(*temp);
			++dest_first;
		}
		else
		{
			// move non-unique elements to the beginning of input range
			for (; temp != first; ++temp)
			{
				if (new_input_end != temp) // watch out for self-move-assignment
					*new_input_end = std::move(*temp);

				++new_input_end;
			}
			first_instance = temp;
		}
	}

	return std::make_pair(new_input_end, dest_first);
}

/**
 * @brief just like std::for_each, but additionally executes nullary function between iterationa,
 * requires binary iterators
 *
 * @details
 * UnaryFunction is called std::distance(first, last) times
 * NullaryFunction is called
 * - 0 times if first == last
 * - std::distance(first, last) - 1 times otherwise
 */
template <typename BidirIt, typename UnaryFunction, typename NullaryFunction>
std::pair<UnaryFunction, NullaryFunction> for_each_and_between(
	BidirIt first,
	BidirIt last,
	UnaryFunction unary_func,
	NullaryFunction nullary_func)
{
	if (first == last)
		return std::make_pair(unary_func, nullary_func);

	const auto pre_last = std::prev(last);
	for (; first != pre_last; ++first)
	{
		unary_func(*first);
		nullary_func();
	}

	unary_func(*first);
	return std::make_pair(unary_func, nullary_func);
}

/**
 * @brief report differences between 2 sorted ranges
 *
 * lhs_first, lhs_last - range 1 (must be sorted)
 * rhs_first, rhs_last - range 2 (must be sorted)
 *
 * comp_eq - accepts (*lhs_it, *rhs_it), implements operator==
 * comp_lt - accepts (*lhs_it, *rhs_it), implements operator<
 *
 * lhs_only_f - called with (*lhs) when it exists only in range 1
 * rhs_only_f - called with (*rhs) when it exists only in range 2
 *
 * Example - for these ranges with default operators:
 * {'a', 'b',           'd', 'e',      'g', 'h',      'j'}
 * {'a', 'b', 'c', 'c', 'd',      'f', 'g',      'i'     }
 * these will be called:
 * rhs_only_f: c
 * rhs_only_f: c
 * lhs_only_f: e
 * rhs_only_f: f
 * lhs_only_f: h
 * rhs_only_f: i
 * lhs_only_f: j
 */
template <
	typename InputIt1,
	typename InputIt2,
	typename BinaryPredicate1,
	typename BinaryPredicate2,
	typename UnaryFunction1,
	typename UnaryFunction2
>
void diff_report(
	InputIt1 lhs_first,
	InputIt1 lhs_last,
	InputIt2 rhs_first,
	InputIt2 rhs_last,
	BinaryPredicate1 comp_eq,
	BinaryPredicate2 comp_lt,
	UnaryFunction1 lhs_only_f,
	UnaryFunction2 rhs_only_f)
{
	auto lhs_it = lhs_first;
	auto rhs_it = rhs_first;

	while (lhs_it != lhs_last || rhs_it != rhs_last) {
		std::tie(lhs_it, rhs_it) = std::mismatch(lhs_it, lhs_last, rhs_it, rhs_last, comp_eq);

		if (lhs_it == lhs_last) {
			// no more items in lhs, all remaining items in rhs are unique
			for (; rhs_it != rhs_last; ++rhs_it) {
				rhs_only_f(*rhs_it);
			}
		}
		else if (rhs_it == rhs_last) {
			// no more items in rhs, all remaining items in lhs are unique
			for (; lhs_it != lhs_last; ++lhs_it) {
				lhs_only_f(*lhs_it);
			}
		}
		// if control flow reaches here we are in the middle of both sequences
		else if (comp_lt(*lhs_it, *rhs_it)) {
			// lhs < rhs
			while (lhs_it != lhs_last && comp_lt(*lhs_it, *rhs_it)) {
				lhs_only_f(*lhs_it);
				++lhs_it;
			}
		}
		else {
			// lhs > rhs
			while (rhs_it != rhs_last && !comp_lt(*lhs_it, *rhs_it) && !comp_eq(*lhs_it, *rhs_it)) {
				rhs_only_f(*rhs_it);
				++rhs_it;
			}
		}
	}
}

}
