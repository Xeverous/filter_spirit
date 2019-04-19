#include <utility>

namespace fs::utility
{

/**
 * @brief algorithm that moves out unique elements
 *
 * @param first beginning of input range
 * @param last end of input range
 * @param d_first beginning of output range
 * @param p BinaryPredicate used to compare elements (represents operator==)
 * @return pair of: new end of input range, new end of output range
 *
 * @details
 * Precondition: input range must be sorted (only adjacent elements may
 * compare equal), otherwise the behaviour is undefined
 *
 * Complexity: linear with the size of input range
 *
 * Exception safety: none; supplied predicate and move constructor of iterated
 * elements must never throw
 *
 * input range: 1 2 2 2 3 3 4 5 5 6
 * output range: (empty)
 *             ||
 *             ||
 *            \||/
 *             \/
 * input range: 2 2 2 3 3 5 5 X X X
 * output range: 1 4 6
 *
 * to complete the erase-remove idiom (remove elements X which have indeterminate values),
 * after calling this algorithm call input.erase(return_val.first, input.end())
 */
template <typename ForwardIt, typename OutputIt, typename BinaryPredicate>
std::pair<ForwardIt, OutputIt> remove_unique(ForwardIt first, ForwardIt last, OutputIt d_first, BinaryPredicate p)
{
	// the place where we move unique elements: d_first
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
			// move this element to destination range
			*d_first = std::move(*temp);
			++d_first;
		}
		else
		{
			// move copies to the beginning of input range
			for (; temp != first; ++temp)
			{
				*new_input_end = std::move(*temp);
				++new_input_end;
			}
			first_instance = temp;
		}
	}

	return std::make_pair(new_input_end, d_first);
}

}
