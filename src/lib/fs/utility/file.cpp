#include <fs/utility/file.hpp>
#include <fs/log/logger.hpp>

#include <fstream>

namespace fs::utility
{

namespace sfs = std::filesystem;

std::string load_file(const std::filesystem::path& path, std::error_code& ec)
{
	sfs::file_status status = sfs::status(path, ec);
	if (ec)
		return {};

	if (sfs::is_directory(status)) {
		ec = std::make_error_code(std::errc::is_a_directory);
		return {};
	}

	const auto file_size = sfs::file_size(path, ec);
	if (ec)
		return {};

	std::ifstream file(path, std::ios::binary);

	if (!file.good()) {
		ec = std::make_error_code(std::io_errc::stream);
		return {};
	}

	std::string file_contents(file_size, '\0');
	file.read(file_contents.data(), static_cast<std::streamsize>(file_size));
	return file_contents;
}

std::optional<std::string> load_file(const std::filesystem::path& path, log::logger& logger)
{
	std::error_code ec;
	std::string file_contents = load_file(path, ec);

	if (ec) {
		logger.error() << "Failed to load file " << path.generic_string() << ": " << ec.message() << ".\n";
		return std::nullopt;
	}

	return file_contents;
}

std::error_code save_file(const std::filesystem::path& path, std::string_view file_contents)
{
	if (sfs::is_directory(path))
		return std::make_error_code(std::errc::is_a_directory);

	std::ofstream file(path, std::ios::binary | std::ios::trunc);

	if (!file.good())
		return std::make_error_code(std::io_errc::stream);

	file.write(file_contents.data(), static_cast<std::streamsize>(file_contents.size()));
	return {};
}

bool save_file(const std::filesystem::path& path, std::string_view file_contents, log::logger& logger)
{
	if (auto ec = save_file(path, file_contents); ec) {
		logger.error() << "Failed to save file " << path.generic_string() << ": " << ec.message() << ".\n";
		return false;
	}

	return true;
}

bool create_directories(const std::filesystem::path& dirpath, log::logger& logger)
{
	// Note: in case all directories already exist, this functions returns false
	// but reports no error. Therefore, only error code is checked.
	std::error_code ec;
	if (std::filesystem::create_directories(dirpath, ec); ec) {
		logger.error() << "Failed to create " << dirpath.generic_string() << ": " << ec.message() << ".\n";
		return false;
	}

	return true;
}

}
