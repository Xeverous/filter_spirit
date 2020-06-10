#pragma once

#include <Windows.h>

[[noreturn]]
void throw_windows_error(HRESULT hr, const char* what_arg);
