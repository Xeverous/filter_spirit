#include <utility>

namespace fs::utility
{

/**
 * @brief algorithm very similar to std::unique_copy
 * except that duplicates are moved out, not copied
 */
template <typename ForwardIt, typename OutputIt, typename BinaryPredicate>
ForwardIt unique_move(ForwardIt first, ForwardIt last, OutputIt d_first, BinaryPredicate p)
{
	if (first == last)
		return last;

	ForwardIt result = first;
	while (++first != last)
	{
		if (!p(*result, *first) && ++result != first)
		{
			*d_first = std::move(*result);
			++d_first;
			*result = std::move(*first);
		}
	}

	return ++result;
}

/**
 * @brief algorithm that moves out unique elements
 *
 * @param first beginning of input range
 * @param last end of input range
 * @param d_first beginning of output range
 * @param p BinaryPredicate used to compare elements (represents operator==)
 * @return pair of: new end of input range, new end of output range
 *
 * Precondition: input range must be sorted (only adjacent elements may
 * compare equal), otherwise the behaviour is undefined
 *
 * input range: 1 2 2 2 3 3 4 5 5 6
 * output range: (empty)
 *             ||
 *             ||
 *            \||/
 *             \/
 * input range: 2 2 2 3 3 5 5
 * output range: 1 4 6
 */
template <typename ForwardIt, typename OutputIt, typename BinaryPredicate>
std::pair<ForwardIt, OutputIt> remove_unique(ForwardIt first, ForwardIt last, OutputIt d_first, BinaryPredicate p)
{
	// the place where we move unique elements: d_first
	// the place where we move repeated elements:
	ForwardIt rep_last = first;

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
				*rep_last = std::move(*temp);
				++rep_last;
			}
			first_instance = temp;
		}
	}

	return std::make_pair(rep_last, d_first);
}

}
