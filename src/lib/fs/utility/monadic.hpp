#pragma once

#include <boost/optional.hpp>

#include <optional>
#include <utility>

namespace fs::utility {

template <typename T>
std::optional<T> to_std_optional(const boost::optional<T>& opt)
{
	if (opt)
		return *opt;
	else
		return std::nullopt;
}

template <typename T>
std::optional<T> to_std_optional(boost::optional<T>&& opt)
{
	if (opt)
		return *std::move(opt);
	else
		return std::nullopt;
}

template <typename T>
boost::optional<T> to_boost_optional(const std::optional<T>& opt)
{
	if (opt)
		return *opt;
	else
		return boost::none;
}

template <typename T>
boost::optional<T> to_boost_optional(std::optional<T>&& opt)
{
	if (opt)
		return *std::move(opt);
	else
		return boost::none;
}

}
