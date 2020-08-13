#pragma once

#include <fs/log/logger.hpp>

#include <mutex>
#include <type_traits>
#include <utility>

namespace fs::log {

namespace impl {

	template <typename T>
	class thread_safe_logger_base
	{
	public:
		thread_safe_logger_base(T logger = T{})
		: _logger(std::move(logger))
		{
		}

	protected:
		T _logger;
	};

	template <typename T>
	class thread_safe_logger_base<T&>
	{
		thread_safe_logger_base(T& l)
		: _logger(l)
		{
		}

	protected:
		T _logger;
	};

}

template <typename T>
class thread_safe_logger : impl::thread_safe_logger_base<T>, public logger
{
public:
	static_assert(std::is_base_of_v<logger, std::remove_reference_t<T>>);

	using impl::thread_safe_logger_base<T>::thread_safe_logger_base;

	void begin_logging() override
	{
		_mtx.lock();
	}

	void end_logging() override
	{
		_mtx.unlock();
	};

	void add(std::string_view text) override
	{
		this->_logger.add(text);
	}

	void add(char character) override
	{
		this->_logger.add(character);
	}

	void add(std::int64_t number) override
	{
		this->_logger.add(number);
	}

	void add(std::uint64_t number) override
	{
		this->_logger.add(number);
	}

	template <typename F>
	auto operator()(F f)
	{
		auto _ = std::lock_guard<std::mutex>(_mtx);
		return f(this->_logger);
	}

	template <typename F>
	auto operator()(F f) const
	{
		auto _ = std::lock_guard<std::mutex>(_mtx);
		return f(this->_logger);
	}

protected:
	void begin_message(severity s) override
	{
		this->_logger.begin_message(s);
	}

	void end_message() override
	{
		this->_logger.end_message();
	}

private:
	mutable std::mutex _mtx;
};

}
