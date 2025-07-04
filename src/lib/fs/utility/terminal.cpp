#include <fs/utility/terminal.hpp>

#ifdef FILTER_SPIRIT_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <sys/ioctl.h>
	#include <stdio.h>
	#include <unistd.h>
#endif

namespace fs::utility {

#ifdef FILTER_SPIRIT_PLATFORM_WINDOWS
std::optional<int> get_terminal_width()
{
	// https://stackoverflow.com/a/6815842/4818802
	// https://stackoverflow.com/a/12642749/4818802
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi) == FALSE)
		return std::nullopt;

	return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}
#elif defined(FILTER_SPIRIT_PLATFORM_EMSCRIPTEN)
std::optional<int> get_terminal_width()
{
	return std::nullopt;
}
#else
std::optional<int> get_terminal_width()
{
	// Note: some terminals might provide environmental variables like COLUMNS
	// but they are not being inherited by processes in the terminal.
	// Thus, ioctl is preferred to getenv.

	// https://stackoverflow.com/a/1022961/4818802
	winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != 0)
		return std::nullopt;

	return w.ws_col;
}
#endif

}
