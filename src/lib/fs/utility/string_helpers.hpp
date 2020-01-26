#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>

namespace fs::utility
{

[[nodiscard]] std::string ptime_to_pretty_string(boost::posix_time::ptime time);

}
