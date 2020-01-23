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

#include <limits>
#include <memory>
#include <future>
#include <stdexcept>
#include <string_view>

namespace
{

// based on:
// https://www.boost.org/doc/libs/master/libs/beast/doc/html/beast/examples.html#beast.examples.clients

using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

using response_type = http::response_parser<http::string_body>;

class session: public std::enable_shared_from_this<session>
{
public:
	explicit
	session(boost::asio::io_context& ioc, ssl::context& ctx)
	: resolver(ioc), stream(ioc, ctx)
	{
	}

	[[nodiscard]] std::future<fs::network::result_type>
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
	http::request<http::empty_body> request; // HTTP GET request
	response_type response;                  // returned HTTP content
	std::vector<std::string> targets;        // array of links
	fs::network::result_type results;        // array of downloaded data
	std::promise<fs::network::result_type> promise;
};


std::future<fs::network::result_type> session::async_http_get(
	const char* host,
	std::vector<std::string> targets)
{
	// save targets that will be used in each request+response pair
	results.reserve(targets.size());
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
	if (results.size() == targets.size()) {
		// we are done, we downloaded from all targets
		promise.set_value(std::move(results));
		results.clear();

		// Gracefully close the stream
		stream.async_shutdown(
			[self = shared_from_this()](boost::system::error_code ec) {
				self->on_shutdown(ec);
			});
		return;
	}

	const auto& current_target = targets[results.size()];
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

	// We want unlimited body size
	// Some of the data we get is too large for default buffer size (8 MB)
	// See https://stackoverflow.com/questions/50348516/boost-beast-message-with-body-limit
	response.body_limit(std::numeric_limits<std::uint64_t>::max());

	// Receive the HTTP response
	http::async_read(
		stream,
		buffer,
		response,
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

	// move the data from the response to our results
	results.resize(results.size() + 1);
	results.back() = std::move(response.get().body());

	next_request();
}

void session::on_shutdown(boost::system::error_code ec)
{
	if (
		// Rationale:
		// http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
		ec == boost::asio::error::eof
		// Technically, we should not ignore this error but unfortunately some services (including poe.ninja's
		// Cloudflare backend) shutdowns abruptly as a way to save bandwidth. This very shady optimization is
		// seen from the client side as an SSL short read which could be a symptom of a man-in-the-middle attack
		// trying to hijack the TCP connection. In HTTP 1.0, this could be used to exploit some implicit assumptions
		// when the connection closes. As of HTTP 1.1, there are no known exploits because 1.1 has no on-close assumptions.
		// See https://github.com/boostorg/beast/issues/38 for more details on the problem.
		// If you use a different networking library and do not get this error, report this as a security issue anyway.
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

std::future<fs::network::result_type>
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
		logger.info() << "downloading from " << host << t;
	}
}

}
