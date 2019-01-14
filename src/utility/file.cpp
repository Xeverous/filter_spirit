#include "utility/file.hpp"
#include <fstream>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>

namespace fs::utility
{

bool file_exists(const char* path)
{
	struct stat buffer;
	return stat(path, &buffer) == 0;
}

std::optional<std::string> load_file(const char* file_path)
{
	std::ifstream file(file_path);

	if (!file.good())
		return {};

	return std::string(std::istreambuf_iterator<char>{file}, {});
}

}
