#include <fs/network/url_encode.hpp>

#include <utility>

namespace
{

std::pair<unsigned char, unsigned char> hexchar(unsigned char c)
{
	unsigned char hex1 = c / 16;
	unsigned char hex2 = c % 16;
	hex1 += hex1 <= 9 ? '0' : 'a' - 10;
	hex2 += hex2 <= 9 ? '0' : 'a' - 10;
	return {hex1, hex2};
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
