#pragma once

#include <future>
#include <chrono>
#include <mutex>

// async stuff utils

namespace fs::utility {

template <typename T>
bool is_ready(const std::future<T>& future)
{
	return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

namespace impl {

template <typename T>
class monitor_base
{
public:
	monitor_base(T t = T{})
	: t(std::move(t))
	{
	}

protected:
	mutable T t;
};

template <typename T>
class monitor_base<T&>
{
public:
	monitor_base(T& t)
	: t(t)
	{
	}

protected:
	T& t;
};

}

// simple thread-safety wrapper that uses a mutex whenever stored object is used
// https://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Herb-Sutter-Concurrency-and-Parallelism
// this one is not exactly the same as in the presentation because reference members can not be declared
// mutable, so inheritance is used for implementation details
template <typename T>
class monitor : public impl::monitor_base<T>
{
public:
	using impl::monitor_base<T>::monitor_base;

	template <typename F>
	auto operator()(F f) const
	{
		auto _ = std::lock_guard<std::mutex>(m);
		return f(this->t);
	}

private:
	mutable std::mutex m;
};

}
