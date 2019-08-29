#include <iostream>
#include <cstdlib>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include "rplib/log.hpp"
#include "config.hpp"
#include "json.hpp"
#include "fs.hpp"

using std::cout;
using std::string;
using std::to_string;
using boost::lexical_cast;
namespace po = boost::program_options;

string const DEFAULT_CONFIG_FILE_NAME = "rplay.conf";
unsigned short DEFAULT_PORT = 13333;

config::config()
	: media_home{home_dir() + "/Music"}
	, port{DEFAULT_PORT}
{}

config::config(int argc, char * argv[])
	: config{}
{
	string const port_help = "server port number, " + to_string(DEFAULT_PORT) + " as default";

	po::options_description desc{"rplay options"};
	desc.add_options()
		("help", "produce help messages")
		("library", po::value<string>(), "path to media library")
		("port", po::value<unsigned short>(), port_help.c_str());

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		cout << desc << "\n";
		exit(1);
	}

	fs::path conf_path{fs::path{getenv("HOME")} / DEFAULT_CONFIG_FILE_NAME};
	if (!fs::exists(conf_path))
		conf_path = DEFAULT_CONFIG_FILE_NAME;

	jtree root;
	try {
		root = read_json_file(conf_path.string());
	}
	catch (std::exception const & e) {
		LOG(warning) << e.what();
	}

	if (vm.count("library"))
		media_home = vm["library"].as<string>();
	else
		media_home = root.get<string>("rplay.media_home", media_home);

	if (vm.count("port"))
		port = vm["port"].as<unsigned short>();
	else
		port = root.get<unsigned short>("rplay.port", port);

	log_file = root.get<string>("rplay.log_file", string{});
}
