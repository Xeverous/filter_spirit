#include <fs/utility/string_helpers.hpp>

namespace fs::utility
{

[[nodiscard]] std::string ptime_to_pretty_string(boost::posix_time::ptime time)
{
	if (time.is_special())
		return "(none)";

	std::string result = to_iso_extended_string(time.date()); // YYYY-MM-DD
	result += ' ';

	const auto append_num = [&](auto num) {
		if (num < 10)
			result += '0';

		result += std::to_string(num);
	};

	// HH:MM:SS
	append_num(time.time_of_day().hours());
	result += ':';
	append_num(time.time_of_day().minutes());
	result += ':';
	append_num(time.time_of_day().seconds());

	return result;
}

}
