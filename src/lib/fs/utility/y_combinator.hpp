#include <utility>
#include <type_traits>

namespace fs::utility {

/*
 * Y-combinator - make a recursive functor that
 * gets a reference to itself inside own call operator
 *
 * Example usage:
 *
 * auto y = make_y_combinator([](const auto& self)
 *     { std::cout << "my address is " << &self << '\n'; }
 * );
 *
 * y();
 */

template <typename F>
struct y_combinator
{
	template <typename... Args>
	decltype(auto) operator()(Args&&... args) const
	{
		return f(f, std::forward<Args>(args)...);
	}

	template <typename... Args>
	decltype(auto) operator()(Args&&... args)
	{
		return f(f, std::forward<Args>(args)...);
	}

	F f;
};

template <typename F>
auto make_y_combinator(F&& f)
{
    return y_combinator<std::decay_t<F>>{std::forward<F>(f)};
}

}
