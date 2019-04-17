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

}
