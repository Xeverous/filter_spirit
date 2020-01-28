#include <fs/utility/file.hpp>
#include <fs/log/logger.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>

namespace fs::utility
{

namespace bfs = boost::filesystem;

std::string load_file(const boost::filesystem::path& path, std::error_code& ec)
{
	boost::system::error_code bec;
	bfs::file_status status = bfs::status(path, bec);

	if (bec) {
		ec = static_cast<std::error_code>(bec);
		return {};
	}

	if (bfs::is_directory(status)) {
		//ec = boost::system::errc::make_error_code(boost::system::errc::is_a_directory);
		ec = std::make_error_code(std::errc::is_a_directory);
		return {};
	}

	const auto file_size = bfs::file_size(path, bec);
	if (ec) {
		ec = static_cast<std::error_code>(bec);
		return {};
	}

	bfs::ifstream file(path, std::ios::binary);

	if (!file.good()) {
		ec = std::make_error_code(std::io_errc::stream);
		return {};
	}

	std::string file_contents(file_size, '\0');
	file.read(file_contents.data(), file_size);
	return file_contents;
}

std::optional<std::string> load_file(const boost::filesystem::path& path, log::logger& logger)
{
	std::error_code ec;
	std::string file_contents = load_file(path, ec);

	if (ec) {
		logger.error() << "failed to load file " << path.generic_string() << ": " << ec.message() << '\n';
		return std::nullopt;
	}

	return file_contents;
}

std::error_code save_file(const boost::filesystem::path& path, std::string_view file_contents)
{
	if (bfs::is_directory(path))
		return std::make_error_code(std::errc::is_a_directory);

	bfs::ofstream file(path, std::ios::binary | std::ios::trunc);

	if (!file.good())
		return std::make_error_code(std::io_errc::stream);

	file.write(file_contents.data(), file_contents.size());
	return {};
}

bool save_file(const boost::filesystem::path& path, std::string_view file_contents, log::logger& logger)
{
	if (auto ec = save_file(path, file_contents); ec) {
		logger.error() << "failed to save file " << path.generic_string() << ": " << ec.message() << '\n';
		return false;
	}

	return true;
}

bool create_directory_if_not_exists(const boost::filesystem::path& dirpath, log::logger& logger)
{
	boost::system::error_code ec;
	const auto status = bfs::status(dirpath, ec);
	if (!status_known(status)) {
		logger.error() << "failed to stat path " << dirpath.generic_string()
			<< ": " << ec.message() << '\n';
		return false;
	}

	if (!bfs::exists(status)) {
		logger.info() << "directory " << dirpath.generic_string() << " does not exist, creating\n";

		if (!boost::filesystem::create_directory(dirpath, ec)) {
			logger.error() << "failed to create " << dirpath.generic_string()
				<< ": " << ec.message() << '\n';
			return false;
		}
	}

	return true;
}

}
