#include <fs/network/poe_ninja/api_data_adapted.hpp>
#include <fs/log/logger.hpp>
#include <fs/utility/file.hpp>

#include <utility>

namespace fs::network::poe_ninja
{

namespace {

std::string make_filename(const char* name)
{
	return std::string(name) + ".json";
}

}

namespace poe1 {

int api_item_price_data::num_files()
{
	return boost::fusion::result_of::size<api_item_price_data>::value;
}

bool api_item_price_data::save(const std::filesystem::path& directory, log::logger& logger) const
{
	bool success = true;
	for_each_file([&](const char* name, bool /* is_currency */, const std::string& content) {
		if (!utility::save_file(directory / make_filename(name), content, logger))
			success = false;
	});

	return success;
}

bool api_item_price_data::load(const std::filesystem::path& directory, log::logger& logger)
{
	bool success = true;
	for_each_file([&](const char* name, bool /* is_currency */, std::string& content) {
		std::optional<std::string> maybe_content = utility::load_file(directory / make_filename(name), logger);

		if (maybe_content)
			content = std::move(*maybe_content);
		else
			success = false;
	});

	return success;
}

} // namespace poe1

namespace poe2 {

int api_item_price_data::num_files()
{
	return boost::fusion::result_of::size<api_item_price_data>::value;
}

bool api_item_price_data::save(const std::filesystem::path& directory, log::logger& logger) const
{
	bool success = true;
	for_each_file([&](const char* name, bool /* is_currency */, const std::string& content) {
		if (!utility::save_file(directory / make_filename(name), content, logger))
			success = false;
	});

	return success;
}

bool api_item_price_data::load(const std::filesystem::path& directory, log::logger& logger)
{
	bool success = true;
	for_each_file([&](const char* name, bool /* is_currency */, std::string& content) {
		std::optional<std::string> maybe_content = utility::load_file(directory / make_filename(name), logger);

		if (maybe_content)
			content = std::move(*maybe_content);
		else
			success = false;
	});

	return success;
}

} // namespace poe2

}
