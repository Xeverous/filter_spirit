#include <fs/utility/algorithm.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

class algorithm_fixture
{
	// nothing for now
};

namespace tt = boost::test_tools;

BOOST_FIXTURE_TEST_SUITE(algorithm_suite, algorithm_fixture)

	// tests fs::utility::remove_unique
	class remove_unique_fixture
	{
	protected:
		std::vector<int> run_remove_unique(std::vector<int>& input)
		{
			const auto initial_input_size = input.size();
			BOOST_TEST_REQUIRE(std::is_sorted(input.begin(), input.end()), "input data to the test must be sorted");

			std::vector<int> output;
			const auto new_end_of_input = fs::utility::remove_unique(
				input.begin(),
				input.end(),
				std::back_inserter(output),
				std::equal_to{}).first;

			BOOST_TEST_REQUIRE((input.begin() <= new_end_of_input));
			BOOST_TEST_REQUIRE((new_end_of_input <= input.end()));
			input.erase(new_end_of_input, input.end());

			BOOST_TEST_REQUIRE(input.size() + output.size() == initial_input_size);

			return output;
		}
	};

	BOOST_FIXTURE_TEST_SUITE(remove_unique_suite, remove_unique_fixture)

		BOOST_AUTO_TEST_CASE(empty_input)
		{
			std::vector<int> input;
			const std::vector<int> output = run_remove_unique(input);

			BOOST_TEST(input.empty());
			BOOST_TEST(output.empty());
		}

		BOOST_AUTO_TEST_CASE(single_element)
		{
			const std::vector<int> initial_input = { 1 };
			std::vector<int> input = initial_input;
			const std::vector<int> output = run_remove_unique(input);

			BOOST_TEST(input.empty());
			BOOST_TEST(output == initial_input, tt::per_element());
		}

		BOOST_AUTO_TEST_CASE(two_same_elements)
		{
			const std::vector<int> initial_input = { 1, 1 };
			std::vector<int> input = initial_input;
			const std::vector<int> output = run_remove_unique(input);

			BOOST_TEST(output.empty());
			BOOST_TEST(input == initial_input, tt::per_element());
		}

		BOOST_AUTO_TEST_CASE(two_different_elements)
		{
			const std::vector<int> initial_input = { 1, 2 };
			std::vector<int> input = initial_input;
			const std::vector<int> output = run_remove_unique(input);

			BOOST_TEST(input.empty());
			BOOST_TEST(output == initial_input, tt::per_element());
		}

		BOOST_AUTO_TEST_CASE(only_unique_elements)
		{
			const std::vector<int> initial_input = { 1, 2, 3, 4, 5 };
			std::vector<int> input = initial_input;
			const std::vector<int> output = run_remove_unique(input);

			BOOST_TEST(input.empty());
			BOOST_TEST(output == initial_input, tt::per_element());
		}

		BOOST_AUTO_TEST_CASE(only_repeated_elements)
		{
			const std::vector<int> initial_input = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5 };
			std::vector<int> input = initial_input;
			const std::vector<int> output = run_remove_unique(input);

			BOOST_TEST(output.empty());
			BOOST_TEST(input == initial_input, tt::per_element());
		}

		BOOST_AUTO_TEST_CASE(input_ends_in_repeated_elements)
		{
			std::vector<int> input = { 1, 2, 2, 2, 3, 4, 5, 5, 5, 5, 6, 6 };
			const std::vector<int> output = run_remove_unique(input);

			BOOST_TEST(output == std::vector<int>({ 1, 3, 4 }), tt::per_element());
			BOOST_TEST(input == std::vector<int>({ 2, 2, 2, 5, 5, 5, 5, 6, 6 }), tt::per_element());
		}

		BOOST_AUTO_TEST_CASE(input_ends_in_unique_elements)
		{
			std::vector<int> input = { 1, 2, 2, 2, 3, 4, 4, 5, 6, 7 };
			const std::vector<int> output = run_remove_unique(input);

			BOOST_TEST(output == std::vector<int>({ 1, 3, 5, 6, 7 }), tt::per_element());
			BOOST_TEST(input == std::vector<int>({ 2, 2, 2, 4, 4 }), tt::per_element());
		}

		BOOST_AUTO_TEST_CASE(input_contains_multiple_siblings)
		{
			std::vector<int> input = { 1, 2, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 8, 9, 10, 10, 11, 11, 11 };
			const std::vector<int> output = run_remove_unique(input);

			BOOST_TEST(output == std::vector<int>({ 1, 2, 3, 7, 8, 9 }), tt::per_element());
			BOOST_TEST(input == std::vector<int>({ 4, 4, 4, 5, 5, 5, 6, 6, 10, 10, 11, 11, 11 }), tt::per_element());
		}

	BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
