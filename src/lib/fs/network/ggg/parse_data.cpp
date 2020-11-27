#include <fs/network/ggg/parse_data.hpp>
#include <fs/network/exceptions.hpp>

#include <nlohmann/json.hpp>

namespace
{

bool has_rule(const nlohmann::json& league_entry, const char* rule_name)
{
	auto& rules = league_entry.at("rules");
	for (const auto& rule : rules) {
		if (rule.at("id") == rule_name)
			return true;
	}

	return false;
}

bool is_hardcore(const nlohmann::json& league_entry)
{
	return has_rule(league_entry, "Hardcore");
}

bool is_ssf(const nlohmann::json& league_entry)
{
	return has_rule(league_entry, "NoParties");
}

} // namespace

namespace fs::network::ggg
{

std::vector<lang::league> parse_league_info(const api_league_data& leagues)
{
	nlohmann::json json = nlohmann::json::parse(leagues.leagues_json);

	if (!json.is_array()) // C++20: [[unlikely]]
		throw network::json_parse_error("league JSON must be an array but it is not");

	std::vector<lang::league> result;
	result.reserve(json.size());

	for (const auto& item : json) {
		if (is_ssf(item))
			continue; // skip SSF leagues - trade is not possible in them

		result.push_back(lang::league{
			item.at("id").get<std::string>(),
			is_hardcore(item)});
	}

	return result;
}

}
