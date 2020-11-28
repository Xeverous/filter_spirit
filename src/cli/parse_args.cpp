#include "parse_args.hpp"
#include "core.hpp"

#include <fs/version.hpp>
#include <fs/log/console_logger.hpp>
#include <fs/lang/market/item_price_data.hpp>
#include <fs/network/curl/libcurl.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/optional.hpp>

#include <cstdlib>
#include <iostream>
#include <exception>
#include <string>

namespace
{

void print_help(const boost::program_options::options_description& options)
{
	std::cout <<
		"Filter Spirit - advanced item filter generator for Path of Exile game client\n\n"
		"Usage:\n"
		"./filter_spirit_cli <data_obtaining_option> [cache_option]... [-g input_path output_path [generation_option]...]\n"
		"./filter_spirit_cli <generic_option>\n"
		"\n"
		"Examples:\n"
		"generate/refresh an item filter (using data not older than 2 hours):\n"
		"./filter_spirit_cli -n \"Standard\" -a 120 -g filter_template.txt output.filter\n"
		"download API data for future reuse:\n"
		"./filter_spirit_cli -n \"Standard\" -a 0\n"
		"generate/refresh an item filter (using previously saved data):\n"
		"./filter_spirit_cli -r cache/ninja_Standard -g filter_template.txt output.filter\n";

	options.print(std::cout);

	std::cout <<
		"\n"
		"Filter Spirit is maintained by Xeverous\n"
		"view documentation and report bugs/feature requests/questions on github.com/Xeverous/filter_spirit\n"
		"contact: /u/Xeverous on reddit, Xeverous#2151 on Discord, pathofexile.com/account/view-profile/Xeverous\n";
}

}

int run(int argc, char* argv[])
{
	// libcurl requires global setup + cleanup before using it
	fs::network::curl::libcurl libcurl;

	fs::log::console_logger logger;

	try {
		namespace po = boost::program_options;

		boost::optional<std::string> download_league_name_watch;
		boost::optional<std::string> download_league_name_ninja;
		bool opt_empty_data = false;
		boost::optional<std::string> data_read_dir;
		po::options_description data_obtaining_options("data obtaining options (use 1)");
		data_obtaining_options.add_options()
			("download-watch,w", po::value(&download_league_name_watch)->value_name("LEAGUE"),
				"download newest item price data from api.poe.watch for specified LEAGUE")
			("download-ninja,n", po::value(&download_league_name_ninja)->value_name("LEAGUE"),
				"download newest item price data from poe.ninja/api for specified LEAGUE")
			("empty-data,e",     po::bool_switch(&opt_empty_data),
				"run with no item price data (all price queries will have no results)")
			("read,r", po::value(&data_read_dir)->value_name("DIRPATH"),
				"read item price data (JSON files) from specified directory (ignores age option)")
		;

		int max_age;
		po::options_description cache_options("cache options");
		cache_options.add_options()
			("age,a", po::value(&max_age)->value_name("MINUTES")->default_value(60),
				"max age of cached item price data - too old data will be redownloaded, "
				"otherwise it is read from file cache; set to 0 if you always want to download")
		;

		boost::optional<std::string> opt_proxy;
		boost::optional<std::string> opt_ca_bundle;
		bool no_ssl_verify_peer = false;
		bool no_ssl_verify_host = false;
		long timeout_ms;
		po::options_description networking_options("networking options");
		networking_options.add_options()
			("proxy,x", po::value(&opt_proxy)->value_name("PROXY"),
				("use PROXY for transfers, example values: \"http://my.proxy:80\","
					" \"https://user:password@proxyserver.com:3128\""
					" see https://curl.haxx.se/libcurl/c/CURLOPT_PROXY.html for"
					" more examples, documentation and environmental variables"))
			("ca-bundle,b", po::value(&opt_ca_bundle)->value_name("FILEPATH"),
				(fs::network::strings::ca_bundle_desc))
			("no-ssl-peer", po::bool_switch(&no_ssl_verify_peer),
				("do not verify the authenticity of the peer's certificate"))
			("no-ssl-host", po::bool_switch(&no_ssl_verify_host),
				("do not verify certificate's name against host name"))
			("timeout,t", po::value(&timeout_ms)->value_name("MILLISECONDS")->default_value(5000),
				("timeout for networking operations, 0 means never timeout"))
		;

		bool opt_generate = false;
		fs::generator::settings st;
		po::options_description generation_options("generation options");
		generation_options.add_options()
			("generate,g",  po::bool_switch(&opt_generate), "generate an item filter")
			("print-ast,p", po::bool_switch(&st.print_ast), "print abstract syntax tree (for debug purposes)")
			("stop-on-error", po::bool_switch(&st.compile_settings.error_handling.stop_on_error),
				"stop on first error")
			("warning-is-error", po::bool_switch(&st.compile_settings.error_handling.treat_warnings_as_errors),
				"treat warnings as errors")
		;

		boost::optional<std::string> input_path;
		boost::optional<std::string> output_path;
		constexpr auto input_path_str = "input-path";
		constexpr auto output_path_str = "output-path";
		po::options_description positional_options("positional arguments required by -g (argument names not required)");
		positional_options.add_options()
			(input_path_str,  po::value(&input_path)->value_name("FILEPATH"),  "FILEPATH to filter template file")
			(output_path_str, po::value(&output_path)->value_name("FILEPATH"), "FILEPATH where to generate the filter")
		;

		bool opt_list_leagues = false;
		bool opt_help = false;
		bool opt_version = false;
		boost::optional<std::string> info_path;
		std::vector<std::string> compare_paths;
		po::options_description generic_options("generic options (use 1)");
		generic_options.add_options()
			("list-leagues,l", po::bool_switch(&opt_list_leagues), "download and list available leagues")
			("help,h",         po::bool_switch(&opt_help),    "print this message")
			("version,v",      po::bool_switch(&opt_version), "print version number")
			("info,i",         po::value(&info_path)->value_name("DIRPATH"), "show information about given item price data save")
			("compare,c",      po::value(&compare_paths)->multitoken()->value_name("DIRPATH DIRPATH"),
				"compare single-property items (cards, oils, scarabs, fossils, ...) in 2 price data saves")
		;

		po::positional_options_description positional_options_description;
		positional_options_description.add(input_path_str, 1).add(output_path_str, 1);

		po::options_description all_options;
		all_options
			.add(data_obtaining_options)
			.add(cache_options)
			.add(networking_options)
			.add(generation_options)
			.add(positional_options)
			.add(generic_options);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(all_options).positional(positional_options_description).run(), vm);
		po::notify(vm);

		if (argc == 0 || argc == 1) {
			// later: run with GUI
			// now: print help and exit
			print_help(all_options);
			return EXIT_SUCCESS;
		}

		// running through the command line
		fs::network::download_settings download_settings;
		download_settings.timeout_milliseconds = timeout_ms;
		download_settings.ssl_verify_host = !no_ssl_verify_host;
		download_settings.ssl_verify_peer = !no_ssl_verify_peer;

		if (opt_ca_bundle)
			download_settings.ca_info_path = *opt_ca_bundle;

		if (opt_proxy)
			download_settings.proxy = *opt_proxy;

		if (opt_help) {
			print_help(all_options);
			return EXIT_SUCCESS;
		}

		if (opt_version) {
			std::cout << to_string(fs::version::current()) << '\n';
			return EXIT_SUCCESS;
		}

		if (opt_list_leagues) {
			list_leagues(download_settings, logger);
			return EXIT_SUCCESS;
		}

		if (info_path) {
			return print_item_price_report(*info_path, logger);
		}

		if (!compare_paths.empty()) {
			return compare_data_saves(compare_paths, logger);
		}

		const auto item_price_report = [&]() -> std::optional<fs::lang::market::item_price_report> {
			if (opt_empty_data) {
				// user explicitly stated to use empty data, some find it useful
				// to write SSF filters where price queries are not used
				return fs::lang::market::item_price_report();
			}
			else {
				return obtain_item_price_report(
					download_league_name_ninja,
					download_league_name_watch,
					boost::posix_time::minutes(max_age),
					download_settings,
					data_read_dir,
					logger);
			}
		}();

		if (opt_generate) {
			if (!generate_item_filter(item_price_report, input_path, output_path, st, logger)) {
				logger.info() << "Filter generation failed.\n";
				return EXIT_FAILURE;
			}
		}
	}
	catch (const std::exception& e) {
		logger.error() << e.what() << '\n';
		return EXIT_FAILURE;
	}
	catch (...) {
		logger.error() << "Unknown error.\n" << fs::log::strings::request_bug_report;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
