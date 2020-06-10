#pragma once

#include <string>

// community-made GGG API documentation:
// https://app.swaggerhub.com/apis-docs/Chuanhsing/poe/1.0.0
namespace fs::network::ggg
{

// https://api.pathofexile.com/leagues
struct api_league_data
{
	std::string leagues_json;
};

}
