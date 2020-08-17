#pragma once

namespace fs::utility {

template <typename T> constexpr
T lerp(T t, T min, T max)
{
	return (static_cast<T>(1) - t) * min + t * max;
}

template <typename T> constexpr
T inverse_lerp(T v, T min, T max)
{
	return (v - min) / (max - min);
}

}
