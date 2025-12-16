#pragma once

#include <cmath>
#include <type_traits>

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

// avoids warning: comparing floating-point with ‘==’ or ‘!=’ is unsafe [-Wfloat-equal]
template <typename FloatingPoint>
bool is_zero(FloatingPoint value)
{
	static_assert(std::is_floating_point_v<FloatingPoint>);
	return std::fpclassify(value) == FP_ZERO;
}

}
