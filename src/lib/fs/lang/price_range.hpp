#pragma once

#include <optional>

namespace fs::lang
{

struct price_range
{
	bool contains(double value) const noexcept
	{
		if (max && *max <= value)
			return false;

		if (min && *min > value)
			return false;

		return true;
	}

	std::optional<double> min;
	std::optional<double> max;
};

}
