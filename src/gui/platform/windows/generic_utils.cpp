#include "platform/windows/generic_utils.hpp"

#include <system_error>
#include <exception>
#include <stdexcept>

void throw_windows_error(HRESULT hr, char const* what_arg)
{
	throw std::system_error(hr, std::system_category(), what_arg);
}
