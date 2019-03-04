#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <memory>
#include <future>
#include <string_view>

namespace fs::network
{

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

class session: public std::enable_shared_from_this<session>
{
public:
	explicit
	session(boost::asio::io_context& ioc, ssl::context& ctx)
	: resolver_(ioc)
	, stream_(ioc, ctx)
	{
	}

	std::future<http::response<http::string_body>> async_http_get(
		const char* host,
		std::string_view port,
		std::string_view target,
		int version);

	void on_resolve(
		boost::system::error_code ec,
		tcp::resolver::results_type results);

	void on_connect(boost::system::error_code ec);

	void on_handshake(boost::system::error_code ec);

	void on_write(
		boost::system::error_code ec,
		std::size_t bytes_transferred);

	void on_read(
		boost::system::error_code ec,
		std::size_t bytes_transferred);

	void on_shutdown(boost::system::error_code ec);

private:
	tcp::resolver resolver_;
	ssl::stream<tcp::socket> stream_;
	boost::beast::flat_buffer buffer_; // (Must persist between reads)
	http::request<http::empty_body> req_;
	http::response<http::string_body> res_;
	std::promise<http::response<http::string_body>> response_promise;
};

}
