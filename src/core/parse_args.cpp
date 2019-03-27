#include "core/parse_args.hpp"
#include "core/version.hpp"
#include "core/core.hpp"

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
		"./filter_spirit <data_obtaining_option> [data_storing_option] [<generation_option> input_path output_path]\n"
		"./filter_spirit <generic_option>\n"
		"\n"
		"Examples:\n"
		"generate/refresh an item filter (using newest available online data):\n"
		"./filter_spirit -d \"Standard\" -g filter_template.txt output.filter\n"
		"download API data for future reuse:\n"
		"./filter_spirit -d \"Standard\" -w .\n"
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

namespace fs::core
{

int run(int argc, char* argv[])
{
	try {
		namespace po = boost::program_options;

		boost::optional<std::string> download_league_name;
		boost::optional<std::string> data_read_dir;
		po::options_description data_obtaining_options("data obtaining options (use 1)");
		data_obtaining_options.add_options()
			("download,d", po::value(&download_league_name), "download newest item price data from api.poe.watch for specified league")
			("read,r", po::value(&data_read_dir), "read item price data (compact.json, itemdata.json) from specified directory")
		;

		boost::optional<std::string> data_write_dir;
		po::options_description data_storing_options("data storing options (use if you would like to save item price data for future use)");
		data_storing_options.add_options()
			("write,w", po::value(&data_write_dir), "save item price data (compact.json, itemdata.json) to specified directory")
		;

		bool opt_generate = false;
		po::options_description generation_options("generation options");
		generation_options.add_options()
			("generate,g", po::bool_switch(&opt_generate), "generate an item filter")
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

		if (argc == 0 || argc == 1)
		{
			// later: run with GUI
			// now: print help and exit
			print_help(all_options);
			return 0;
		}

		// running through the command line

		if (opt_help)
		{
			print_help(all_options);
			return EXIT_SUCCESS;
		}

		if (opt_version)
		{
			std::cout << version::major << '.' << version::minor << '.' << version::patch << '\n';
			return EXIT_SUCCESS;
		}

		if (opt_list_leagues)
		{
			list_leagues();
			return EXIT_SUCCESS;
		}

		std::optional<itemdata::item_price_data> item_price_data;

		if (download_league_name && data_read_dir)
		{
			std::cout << "error: more than 1 data obtaining option specified\n";
			return EXIT_FAILURE;
		}

		if (download_league_name)
		{
			item_price_data = download_item_price_data(*download_league_name);
		}

		if (data_read_dir)
		{
			item_price_data = load_item_price_data(*data_read_dir);
		}

		if (opt_generate)
		{
			if (!item_price_data)
			{
				std::cout << "error: no item price data, giving up on filter generation\n";
				return EXIT_FAILURE;
			}

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

			if (!generate_item_filter(*item_price_data, *input_path, *output_path))
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
