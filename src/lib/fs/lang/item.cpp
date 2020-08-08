#include <fs/lang/item.hpp>
#include <fs/lang/keywords.hpp>
#include <fs/utility/string_helpers.hpp>

#include <utility>

namespace fs::lang {

std::optional<socket_info> to_socket_info(std::string_view str)
{
	str = utility::trim(str, ' ');

	linked_sockets current_group;
	socket_info result;
	bool on_socket = true;

	for (char c : str) {
		if (on_socket) {
			if (c == keywords::rf::r)
				current_group.sockets.push_back(socket_color::r);
			else if (c == keywords::rf::g)
				current_group.sockets.push_back(socket_color::g);
			else if (c == keywords::rf::b)
				current_group.sockets.push_back(socket_color::b);
			else if (c == keywords::rf::w)
				current_group.sockets.push_back(socket_color::w);
			else if (c == keywords::rf::a)
				current_group.sockets.push_back(socket_color::a);
			else if (c == keywords::rf::d)
				current_group.sockets.push_back(socket_color::d);
			else
				return std::nullopt; // invalid character
		}
		else {
			if (c == ' ') {
				// new group will come, push current one
				result.groups.push_back(std::move(current_group));
				current_group.sockets.clear();
			}
			else if (c == '-') {
				// current group will continue, just go on...
			}
			else {
				return std::nullopt; // invalid character
			}
		}

		on_socket = !on_socket;
	}

	if (on_socket)
		return std::nullopt; // input ended on link character (eg "R-G-B-")

	// push any leftover group
	result.groups.push_back(std::move(current_group));
	return result;
}

char to_char(socket_color color)
{
	if (color == socket_color::r)
		return keywords::rf::r;
	if (color == socket_color::g)
		return keywords::rf::g;
	if (color == socket_color::b)
		return keywords::rf::b;
	if (color == socket_color::w)
		return keywords::rf::w;
	if (color == socket_color::a)
		return keywords::rf::a;
	if (color == socket_color::d)
		return keywords::rf::d;

	return '?';
}

std::string to_string(socket_info info)
{
	std::string result;
	result.reserve(11); // longest possible outcome

	bool first_time_group = true;

	for (linked_sockets group : info.groups) {
		if (first_time_group)
			first_time_group = false;
		else
			result.push_back(' ');

		bool first_time_color = true;

		for (socket_color color : group.sockets) {
			if (first_time_color)
				first_time_color = false;
			else
				result.push_back('-');

			result.push_back(to_char(color));
		}
	}

	return result;
}

}
