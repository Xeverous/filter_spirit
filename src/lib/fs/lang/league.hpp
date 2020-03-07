#pragma once

#include <string>

namespace fs::lang
{

struct league
{
	int id;
	std::string name;         // name used by the API
	std::string display_name; // name intended for display to users
	bool is_hardcore;
	bool is_upcoming;
	bool is_active;
	bool is_event;
	bool is_challenge;
};

}
