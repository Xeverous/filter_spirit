#include "core/core.hpp"
#include "core/version.hpp"
#include "core/application_logic.hpp"
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
		bool opt_help = false;
		bool opt_version = false;
		bool opt_download = false;
		boost::optional<std::string> input_path;
		boost::optional<std::string> output_path;

		constexpr auto input_path_str = "input-path";
		constexpr auto output_path_str = "output-path";

		namespace po = boost::program_options;

		po::options_description main_options("main options");
		main_options.add_options()
			("download,d", po::value(&opt_download), "download item price data from api.poe.watch\n"
				"by default data is loaded from last saved download (if such exists)")
		;

		po::options_description generic_options("generic options");
		generic_options.add_options()
			("help,h",     po::bool_switch(&opt_help),    "print this message")
			("version,v",  po::bool_switch(&opt_version), "print version number")
		;

		po::options_description positional_options("positional options (option names not required)");
		positional_options.add_options()
			(input_path_str,  po::value(&input_path),  "file path to Filter Spirit source")
			(output_path_str, po::value(&output_path), "file path where to generate the filter")
		;

		po::positional_options_description positional_options_description;
		positional_options_description.add(input_path_str, 1).add(output_path_str, 1);

		po::options_description all_options;
		all_options.add(main_options).add(positional_options).add(generic_options);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(all_options).positional(positional_options_description).run(), vm);
		po::notify(vm);

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

		if (argc == 0 || argc == 1)
		{
			// later: run with GUI
			// now: print help and exit
			std::cout << all_options << '\n';
			return 0;
		}

		// running through the command line

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

		application_logic al;
		if (!al.load_source_file(*input_path))
		{
			std::cout << "error: could not load the input file\n";
			return EXIT_FAILURE;
		}

		if (!al.generate_filter(*output_path))
		{
			std::cout << "errors occured during filter generation\n";
			return EXIT_FAILURE;
		}
	}
	catch (const std::exception& e) {
		std::cout << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

}
