#pragma once

#include <boost/container/small_vector.hpp>

#include <tuple>
#include <variant>
#include <optional>
#include <exception>
#include <stdexcept>
#include <iterator>
#include <utility>

namespace fs::utility
{

template <typename Log, std::size_t N>
struct outcome_base
{
	using log_container_type = boost::container::small_vector<Log, N>;

	static void move_logs(log_container_type&& from, log_container_type& to)
	{
		to.reserve(to.size() + from.size());
		to.insert(to.end(), std::make_move_iterator(from.begin()), std::make_move_iterator(from.end()));
	}
};

/**
 * @class A type representing a result with some execution log.
 * @details This is similar to Boost.Outcome design, but has multiple
 * differences to accomodate needs of FS.
 *
 * There are no hard invariants, the result can be empty/non-empty
 * independently of the log but it is recommended that users of this
 * class do not leave it in a state with no result and with no log.
 *
 * @tparam Log type of objects that will be continuously accumulated
 * @tparam N number of log objects that will have preallocated storage,
 * without the need to use dynamic allocation.
 * @tparam Results types stored as a result tuple. sizeof...(Results) == 0
 * is allowed. No behavioral changes in such case, result can still
 * exist or not (optional tuple of length 0).
 */
template <typename Log, std::size_t N, typename... Results>
class [[nodiscard]] outcome : public outcome_base<Log, N>
{
public:
	static_assert(!(std::is_void_v<Results> || ...));
	static_assert(!std::is_void_v<Log>);

	using result_type = std::tuple<Results...>;
	using log_container_type = typename outcome_base<Log, N>::log_container_type;

	// deleted on purpose to preserve invariants
	outcome() = delete;

	// success ctors
	outcome(result_type result)
	: _result(std::move(result))
	{
	}

	template <
		typename... Args,
		std::enable_if_t<
			sizeof...(Args) != 0u &&
			sizeof...(Args) == sizeof...(Results) &&
			!std::is_constructible_v<Log, Args...> &&
			std::is_constructible_v<result_type, Args...>
		>* = nullptr
	>
	outcome(Args&&... args)
	: _result(std::forward<Args>(args)...)
	{
	}

	outcome(result_type result, log_container_type logs)
	: _result(std::move(result))
	, _logs(std::move(logs))
	{
	}

	static outcome success()
	{
		static_assert(std::is_default_constructible_v<result_type>);
		return outcome(result_type());
	}

	static outcome success(log_container_type logs)
	{
		static_assert(std::is_default_constructible_v<result_type>);
		return outcome(result_type(), std::move(logs));
	}

	// failure ctors

	outcome(log_container_type logs)
	: _logs(std::move(logs))
	{
	}

	explicit outcome(Log log)
	{
		_logs.push_back(std::move(log));
	}

	template <
		typename... Args,
		std::enable_if_t<
			sizeof...(Args) != 0u &&
			std::is_constructible_v<Log, Args...> &&
			!std::is_constructible_v<result_type, Args...>
		>* = nullptr
	>
	outcome(Args&&... args)
	{
		_logs.emplace_back(std::forward<Args>(args)...);
	}

	// move operations
	outcome(outcome&& other) = default;
	outcome& operator=(outcome&& other) = default;

	bool has_result() const noexcept
	{
		return _result.has_value();
	}

	void destroy_result()
	{
		_result.reset();
	}

	auto& result() &
	{
		static_assert(is_tuple_size_1());
		check_result();

		return std::get<0>(*_result);
	}

	const auto& result() const &
	{
		static_assert(is_tuple_size_1());
		check_result();

		return std::get<0>(*_result);
	}

	auto&& result() &&
	{
		static_assert(is_tuple_size_1());
		check_result();

		return std::get<0>(std::move(*_result));
	}

	const auto&& result() const &&
	{
		static_assert(is_tuple_size_1());
		check_result();

		return std::get<0>(std::move(*_result));
	}

	auto& result_as_tuple() &
	{
		check_result();
		return *_result;
	}

	const auto& result_as_tuple() const &
	{
		check_result();
		return *_result;
	}

	auto&& result_as_tuple() &&
	{
		check_result();
		return std::move(*_result);
	}

	const auto&& result_as_tuple() const &&
	{
		check_result();
		return std::move(*_result);
	}

	outcome& push_log(Log log) &
	{
		_logs.push_back(std::move(log));
		return *this;
	}

	outcome& push_log(log_container_type logs) &
	{
		outcome_base<Log, N>::move_logs(std::move(logs), _logs);
		return *this;
	}

	template <typename... Args>
	outcome& emplace_log(Args&&... args) &
	{
		_logs.emplace_back(std::forward<Args>(args)...);
		return *this;
	}

	template <typename... R>
	outcome<Log, N, Results..., R...> merge_with(outcome<Log, N, R...> rhs) &&
	{
		return merge(std::move(*this), std::move(rhs));
	}

	template <typename F>
	outcome& on_success(F f) &
	{
		static_assert(std::is_invocable_v<F, const Results&...>);

		using invoke_result_t = std::invoke_result_t<F, const Results&...>;
		constexpr bool invoke_result_is_void = std::is_same_v<invoke_result_t, void>;
		constexpr bool invoke_result_is_log  = std::is_same_v<invoke_result_t, Log>;
		constexpr bool invoke_result_is_log_container = std::is_same_v<invoke_result_t, log_container_type>;
		static_assert(invoke_result_is_void || invoke_result_is_log || invoke_result_is_log_container);

		if (!has_result())
			return *this;

		if constexpr (invoke_result_is_void)
			std::apply(f, std::as_const(*_result));
		else
			push_log(std::apply(f, std::as_const(*_result)));

		return *this;
	}

	template <typename... T, typename F>
	outcome<Log, N, T...> map_result(F on_success) &&
	{
		static_assert(std::is_invocable_v<F, Results...>);

		using invoke_result_t = std::invoke_result_t<F, Results...>;
		constexpr bool invoke_result_is_outcome = std::is_same_v<invoke_result_t, outcome<Log, N, T...>>;
		constexpr bool invoke_result_is_tuple   = std::is_same_v<invoke_result_t, result_type>;
		constexpr bool map_to_1 = sizeof...(T) == 1u;
		constexpr bool map_to_0 = sizeof...(T) == 0u;
		static_assert(invoke_result_is_outcome || invoke_result_is_tuple || map_to_1 || map_to_0);

		// unsuccessful: return logs and ignore supplied function
		if (!has_result())
			return outcome<Log, N, T...>(std::move(*this).logs());

		if constexpr (invoke_result_is_outcome) {
			return merge(
				outcome<Log, N>::success(std::move(*this).logs()),
				std::apply(on_success, std::move(*_result)));
		}
		else if constexpr (invoke_result_is_tuple) {
			return outcome<Log, N, T...>(std::apply(on_success, std::move(*_result)), std::move(_logs));
		}
		else if constexpr (map_to_1) {
			static_assert(std::is_same_v<invoke_result_t, T...>);
			return outcome<Log, N, T...>(std::apply(on_success, std::move(*_result)), std::move(_logs));
		}
		else if constexpr (map_to_0) {
			static_assert(std::is_same_v<invoke_result_t, void>);
			std::apply(on_success, std::move(*_result));
			// assume success
			return outcome<Log, N, T...>(std::tuple<T...>{}, std::move(_logs));
		}
		else {
			static_assert(sizeof(F) == 0u, "no matching result type, invalid functor supplied");
		}
	}

	log_container_type& logs() &
	{
		return _logs;
	}

	const log_container_type& logs() const &
	{
		return _logs;
	}

	log_container_type logs() &&
	{
		return std::move(_logs);
	}

	void move_logs_to(log_container_type& other)
	{
		outcome_base<Log, N>::move_logs(std::move(_logs), other);
		_logs.clear(); // do not forget to reset state of a moved-from object
	}

private:
	void check_result() const
	{
		if (!has_result())
			throw std::logic_error("required result where one does not exist");
	}

	static constexpr bool is_tuple_size_1() noexcept
	{
		return std::tuple_size_v<result_type> == 1u;
	}

	std::optional<result_type> _result;
	log_container_type _logs;
};

/**
 * merge 2 outcomes
 *
 * logs: always summed up
 * result: if both are non-empty, they are merged into larger tuple
 * otherwise, the merged result is empty
 */
template <typename Log, std::size_t N, typename... RL, typename... RR>
outcome<Log, N, RL..., RR...> merge(outcome<Log, N, RL...> lhs, outcome<Log, N, RR...> rhs)
{
	if (lhs.has_result() && rhs.has_result()) {
		auto result = outcome<Log, N, RL..., RR...>(
			std::tuple_cat(std::move(lhs.result_as_tuple()), std::move(rhs.result_as_tuple())),
			std::move(lhs).logs());
		result.push_log(std::move(rhs).logs());
		return result;
	}

	auto result = outcome<Log, N, RL..., RR...>(std::move(lhs).logs());
	result.push_log(std::move(rhs).logs());
	return result;
}

}
