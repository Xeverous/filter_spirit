#include <utility>

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

}
