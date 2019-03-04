#include "network/network.hpp"
#include "network/session.hpp"

namespace fs::network
{

std::future<boost::beast::http::response<boost::beast::http::string_body>>
	async_http_get(
		boost::asio::io_context& ioc,
		boost::asio::ssl::context& ctx,
		const char* host,
		std::string_view port,
		std::string_view target)
{
	return std::make_shared<session>(ioc, ctx)->async_http_get(host, port, target, /* HTTP 1.1 */ 11);
}

}
