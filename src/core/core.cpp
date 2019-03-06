#include "core/core.hpp"
#include "core/version.hpp"
#include "core/application_logic.hpp"
#include "network/poe_watch_api.hpp"
#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <cstdlib>
#include <iostream>
#include <exception>
#include <string>

namespace fs::core
{

int run(int argc, char* argv[])
{
	try {
		namespace po = boost::program_options;

		bool opt_list_leagues = false;
		boost::optional<std::string> league_name;
		bool opt_generate = false;
		po::options_description main_options("main options");
		main_options.add_options()
			("list-leagues,l", po::bool_switch(&opt_list_leagues), "list available leagues (this option will query the API)")
			("download,d", po::value(&league_name), "download item price data from api.poe.watch for specified league\n(this option can be combined with -g)")
			("generate,g", po::bool_switch(&opt_generate), "generate an item filter using currently saved item price data")
		;

		boost::optional<std::string> input_path;
		boost::optional<std::string> output_path;
		constexpr auto input_path_str = "input-path";
		constexpr auto output_path_str = "output-path";
		po::options_description positional_options("positional options required by -g (option names not required)");
		positional_options.add_options()
			(input_path_str,  po::value(&input_path),  "file path to Filter Spirit source")
			(output_path_str, po::value(&output_path), "file path where to generate the filter")
		;

		bool opt_help = false;
		bool opt_version = false;
		po::options_description generic_options("generic options");
		generic_options.add_options()
			("help,h",     po::bool_switch(&opt_help),    "print this message")
			("version,v",  po::bool_switch(&opt_version), "print version number")
		;

		po::positional_options_description positional_options_description;
		positional_options_description.add(input_path_str, 1).add(output_path_str, 1);

		po::options_description all_options;
		all_options.add(main_options).add(positional_options).add(generic_options);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(all_options).positional(positional_options_description).run(), vm);
		po::notify(vm);

		if (argc == 0 || argc == 1)
		{
			// later: run with GUI
			// now: print help and exit
			std::cout << all_options << '\n';
			return 0;
		}

		// running through the command line

		if (opt_help)
		{
			std::cout << all_options << '\n';
			return 0;
		}

		if (opt_version)
		{
			std::cout << version::major << '.'  << version::minor << '.'  << version::patch << '\n';
			return EXIT_SUCCESS;
		}

		application_logic al;

		if (opt_list_leagues)
		{
			al.list_leagues();
			return EXIT_SUCCESS;
		}

		if (opt_generate)
		{
			if (!input_path)
			{
				std::cout << "error: no input path given\n";
				return EXIT_FAILURE;
			}

			if (!output_path)
			{
				std::cout << "error: no output path given\n";
				return EXIT_FAILURE;
			}

			if (!league_name)
			{
				std::cout << "error: no league name given\n";
				return EXIT_FAILURE;
			}

			if (!al.generate_filter(*league_name, *input_path, *output_path))
			{
				std::cout << "errors occured during filter generation\n";
				return EXIT_FAILURE;
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cerr << "unknown error occured\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

}
