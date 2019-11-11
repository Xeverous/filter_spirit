#include "parse_args.hpp"
#include "core.hpp"

#include <fs/version.hpp>
#include <fs/log/console_logger.hpp>
#include <fs/lang/item_price_data.hpp>
#include <fs/lang/item_price_metadata.hpp>

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
		"./filter_spirit <data_obtaining_option> [data_storing_option] [-g input_path output_path [generation_option]...]\n"
		"./filter_spirit <generic_option>\n"
		"\n"
		"Examples:\n"
		"generate/refresh an item filter (using newest available online data):\n"
		"./filter_spirit -n \"Standard\" -g filter_template.txt output.filter\n"
		"download API data for future reuse:\n"
		"./filter_spirit -n \"Standard\" -s .\n"
		"generate/refresh an item filter (using previously saved data):\n"
		"./filter_spirit -r . -g filter_template.txt output.filter\n";

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
	fs::log::console_logger logger;

	try {
		namespace po = boost::program_options;

		boost::optional<std::string> download_league_name_watch;
		boost::optional<std::string> download_league_name_ninja;
		boost::optional<std::string> data_read_dir;
		po::options_description data_obtaining_options("data obtaining options (use 1)");
		data_obtaining_options.add_options()
			("download-watch,w", po::value(&download_league_name_watch), "download newest item price data from api.poe.watch for specified league")
			("download-ninja,n", po::value(&download_league_name_ninja), "download newest item price data from poe.ninja/api for specified league")
			("read,r", po::value(&data_read_dir), "read item price data (JSON files) from specified directory")
		;

		boost::optional<std::string> data_save_dir;
		po::options_description data_storing_options("data storing options (use if you would like to save item price data for future use)");
		data_storing_options.add_options()
			("save,s", po::value(&data_save_dir), "save item price data (JSON files) to specified directory (requires download option)")
		;

		bool opt_generate = false;
		bool opt_print_ast = false;
		po::options_description generation_options("generation options");
		generation_options.add_options()
			("generate,g",  po::bool_switch(&opt_generate),  "generate an item filter")
			("print-ast,a", po::bool_switch(&opt_print_ast), "print abstract syntax tree (for debug purposes)")
		;

		boost::optional<std::string> input_path;
		boost::optional<std::string> output_path;
		constexpr auto input_path_str = "input-path";
		constexpr auto output_path_str = "output-path";
		po::options_description positional_options("positional arguments required by -g (argument names not required)");
		positional_options.add_options()
			(input_path_str,  po::value(&input_path),  "file path to filter template source")
			(output_path_str, po::value(&output_path), "file path where to generate the filter")
		;

		bool opt_list_leagues = false;
		bool opt_help = false;
		bool opt_version = false;
		po::options_description generic_options("generic options (use 1)");
		generic_options.add_options()
			("list-leagues,l", po::bool_switch(&opt_list_leagues), "download and list leagues")
			("help,h",         po::bool_switch(&opt_help),    "print this message")
			("version,v",      po::bool_switch(&opt_version), "print version number")
		;

		po::positional_options_description positional_options_description;
		positional_options_description.add(input_path_str, 1).add(output_path_str, 1);

		po::options_description all_options;
		all_options.add(data_obtaining_options).add(data_storing_options).add(generation_options).add(positional_options).add(generic_options);

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

		if (opt_help) {
			print_help(all_options);
			return EXIT_SUCCESS;
		}

		if (opt_version) {
			namespace v = fs::version;
			std::cout << v::major << '.' << v::minor << '.' << v::patch << '\n';
			return EXIT_SUCCESS;
		}

		if (opt_list_leagues) {
			list_leagues(logger);
			return EXIT_SUCCESS;
		}

		std::optional<item_data> data = obtain_item_data(
			download_league_name_ninja, download_league_name_watch, data_read_dir, data_save_dir, logger);

		if (opt_generate) {
			if (!generate_item_filter(data, input_path, output_path, opt_print_ast, logger)) {
				logger.info() << "filter generation failed";
				return EXIT_FAILURE;
			}
		}
	}
	catch (const std::exception& e) {
		logger.error() << e.what();
		return EXIT_FAILURE;
	}
	catch (...) {
		logger.error() << "unknown error";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
