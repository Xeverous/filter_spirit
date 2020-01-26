#include <fs/network/http.hpp>
#include <fs/log/logger.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/system/system_error.hpp>

#include <memory>
#include <future>
#include <utility>
#include <stdexcept>
#include <string_view>

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

// based on:
// https://www.boost.org/doc/libs/master/libs/beast/doc/html/beast/examples.html#beast.examples.clients

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

class session: public std::enable_shared_from_this<session>
{
public:
	explicit
	session(boost::asio::io_context& ioc, ssl::context& ctx)
	: resolver(ioc), stream(ioc, ctx)
	{
	}

	[[nodiscard]] std::future<std::vector<http::response<http::string_body>>>
	async_http_get(
		const char* host,
		std::vector<std::string> targets);

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
	void next_request();

	tcp::resolver resolver;
	ssl::stream<tcp::socket> stream;
	boost::beast::flat_buffer buffer; // (Must persist between reads)
	http::request<http::empty_body> request;
	std::vector<std::string> targets;
	std::vector<http::response<http::string_body>> responses;
	std::promise<std::vector<http::response<http::string_body>>> promise;
};


std::future<std::vector<http::response<http::string_body>>> session::async_http_get(
	const char* host,
	std::vector<std::string> targets)
{
	// save targets that will be used in each request+response pair
	responses.reserve(targets.size());
	this->targets = std::move(targets);

	// Set SNI Hostname (many hosts need this to handshake successfully)
	if (!SSL_set_tlsext_host_name(stream.native_handle(), host)) {
		boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
		throw boost::system::system_error(ec);
	}

	// Set up an HTTP GET request message
	request.version(11); // HTTP 1.1
	request.method(http::verb::get);
	request.set(http::field::host, host);
	request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
	// request.keep_alive(true); // redundant in HTTP 1.1

	// Look up the domain name
	resolver.async_resolve(
		host,
		"443", // default HTTP port
		[self = shared_from_this()]
		(boost::system::error_code ec, tcp::resolver::results_type results) {
			self->on_resolve(ec, results);
		});

	return promise.get_future();
}

void session::on_resolve(
	boost::system::error_code ec,
	tcp::resolver::results_type results)
{
	if (ec)
		throw boost::system::system_error(ec, "could not resolve host name");

	// Make the connection on the IP address we get from a lookup
	boost::asio::async_connect(
		stream.next_layer(),
		results.begin(),
		results.end(),
		std::bind( // lambda does not compile here - beast bug probably (example changed between 1.69 and 1.70)
			&session::on_connect,
			shared_from_this(),
			std::placeholders::_1));
}

void session::on_connect(boost::system::error_code ec)
{
	if (ec)
		throw boost::system::system_error(ec, "could not connect to the host");

	// Perform the SSL handshake
	stream.async_handshake(
		ssl::stream_base::client,
		[self = shared_from_this()](boost::system::error_code ec) {
			self->on_handshake(ec);
		});
}

void session::on_handshake(boost::system::error_code ec)
{
	if (ec)
		throw boost::system::system_error(ec, "could not handshake with the target");

	next_request();
}

void session::next_request()
{
	if (responses.size() == targets.size()) {
		promise.set_value(std::move(responses));
		responses.clear();

		// Gracefully close the stream
		stream.async_shutdown(
			[self = shared_from_this()](boost::system::error_code ec) {
				self->on_shutdown(ec);
			});
		return;
	}

	const auto& current_target = targets[responses.size()];
	request.target(boost::beast::string_view(current_target.data(), current_target.size()));

	// Send the HTTP request to the remote host
	http::async_write(
		stream,
		request,
		[self = shared_from_this()]
		(boost::system::error_code ec, std::size_t bytes_transferred) {
			self->on_write(ec, bytes_transferred);
		});
}

void session::on_write(
	boost::system::error_code ec,
	std::size_t /* bytes_transferred */)
{
	if (ec)
		throw boost::system::system_error(ec, "could not write the request");

	responses.resize(responses.size() + 1);

	// Receive the HTTP response
	http::async_read(
		stream,
		buffer,
		responses.back(),
		[self = shared_from_this()]
		(boost::system::error_code ec, std::size_t bytes_transferred) {
			self->on_read(ec, bytes_transferred);
		});
}

void session::on_read(
	boost::system::error_code ec,
	std::size_t /* bytes_transferred */)
{
	if (ec)
		throw boost::system::system_error(ec, "could not read the request");

	next_request();
}

void session::on_shutdown(boost::system::error_code ec)
{
	if (
		// Rationale:
		// http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
		ec == boost::asio::error::eof
		// rationale: https://github.com/boostorg/beast/issues/38
		|| ec == boost::asio::ssl::error::stream_truncated)
	{
		ec.assign(0, ec.category());
	}

	if (ec) {
		throw boost::system::system_error(ec, "could not shutdown the connection");
	}

	// If we get here then the connection is closed gracefully
}

} // namespace

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

std::future<std::vector<boost::beast::http::response<boost::beast::http::string_body>>>
async_http_get(
	boost::asio::io_context& ioc,
	boost::asio::ssl::context& ctx,
	const char* host,
	std::vector<std::string> targets)
{
	return std::make_shared<session>(ioc, ctx)->async_http_get(host, std::move(targets));
}

void log_download_information(
	const char* host,
	const std::vector<std::string>& targets,
	log::logger& logger)
{
	for (const auto& t : targets) {
		logger.info() << "downloading from " << host << t << '\n';
	}
}

}
