#pragma once

#include <fs/log/logger.hpp>
#include <fs/utility/async.hpp>

namespace fs::log {

// convenience alias
using monitor = utility::monitor<logger&>;

}
