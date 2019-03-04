#include "network/session.hpp"

#include <boost/system/system_error.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>

#include <stdexcept>

namespace fs::network
{

std::future<http::response<http::string_body>> session::async_http_get(
	const char* host,
	std::string_view port,
	std::string_view target,
	int version)
{
	// Set SNI Hostname (many hosts need this to handshake successfully)
	if (!SSL_set_tlsext_host_name(stream_.native_handle(), host))
	{
		boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
		throw boost::system::system_error(ec);
	}

	// Set up an HTTP GET request message
	req_.version(version);
	req_.method(http::verb::get);
	req_.target(boost::beast::string_view(target.data(), target.size()));
	req_.set(http::field::host, host);
	req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

	// Look up the domain name
	resolver_.async_resolve(
		host,
		port,
		std::bind(
			&session::on_resolve,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2));

	return response_promise.get_future();
}

void session::on_resolve(
	boost::system::error_code ec,
	tcp::resolver::results_type results)
{
	if (ec)
		throw boost::system::system_error(ec, "could not resolve host name");

	// Make the connection on the IP address we get from a lookup
	boost::asio::async_connect(
		stream_.next_layer(),
		results.begin(),
		results.end(),
		std::bind(
			&session::on_connect,
			shared_from_this(),
			std::placeholders::_1));
}

void session::on_connect(boost::system::error_code ec)
{
	if (ec)
		throw boost::system::system_error(ec, "could not connect to the host");

	// Perform the SSL handshake
	stream_.async_handshake(
		ssl::stream_base::client,
		std::bind(
			&session::on_handshake,
			shared_from_this(),
			std::placeholders::_1));
}

void session::on_handshake(boost::system::error_code ec)
{
	if (ec)
		throw boost::system::system_error(ec, "could not handshake with the target");

	// Send the HTTP request to the remote host
	http::async_write(stream_, req_,
		std::bind(
			&session::on_write,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2));
}

void session::on_write(
	boost::system::error_code ec,
	std::size_t /* bytes_transferred */)
{
	if (ec)
		throw boost::system::system_error(ec, "could not write the request");

	// Receive the HTTP response
	http::async_read(stream_, buffer_, res_,
		std::bind(
			&session::on_read,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2));
}

void session::on_read(
	boost::system::error_code ec,
	std::size_t /* bytes_transferred */)
{
	if (ec)
		throw boost::system::system_error(ec, "could not read the request");

	response_promise.set_value(std::move(res_));

	// Gracefully close the stream
	stream_.async_shutdown(
		std::bind(
			&session::on_shutdown,
			shared_from_this(),
			std::placeholders::_1));
}

void session::on_shutdown(boost::system::error_code ec)
{
	if (ec == boost::asio::error::eof)
	{
		// Rationale:
		// http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
		ec.assign(0, ec.category());
	}
	if (ec)
	{
		throw boost::system::system_error(ec, "could not shutdown the connection");
	}

	// If we get here then the connection is closed gracefully
}

}
