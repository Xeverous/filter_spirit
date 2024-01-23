#include <fs/network/url_encode.hpp>

#include <utility>

namespace
{

std::pair<char, char> hexchar(char c)
{
	unsigned char hex1 = static_cast<unsigned char>(c) / 16u;
	unsigned char hex2 = static_cast<unsigned char>(c) % 16u;
	hex1 += hex1 <= 9 ? '0' : 'a' - 10;
	hex2 += hex2 <= 9 ? '0' : 'a' - 10;
	return {static_cast<char>(hex1), static_cast<char>(hex2)};
}

}

namespace fs::network
{

void url_encode(std::string_view str, std::string& buffer)
{
	buffer.clear();
	for (char c : str) {
		if (
			(c >= '0' && c <= '9') ||
			(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			c == '-' || c == '_'  || c == '.' || c == '!' || c == '~' ||
			c == '*' || c == '\'' || c == '(' || c == ')')
		{
			buffer.push_back(c);
		}
		else if (c == ' ') {
			buffer.push_back('+');
		}
		else {
			buffer.push_back('%');
			const auto pair = hexchar(c);
			buffer.push_back(pair.first);
			buffer.push_back(pair.second);
		}
	}
}

std::string url_encode(std::string_view str)
{
	std::string result;
	url_encode(str, result);
	return result;
}

}
