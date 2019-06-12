#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include "rplib/log.hpp"
#include "config.hpp"
#include "json.hpp"
#include "fs.hpp"

using std::string;
using boost::lexical_cast;

string const DEFAULT_CONFIG_FILE_NAME = "rplay.conf";

config::config()
	: media_home{home_dir() + "/Music"}
	, port{13333}
{}

config::config(int argc, char * argv[])
	: config{}
{
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

	if (argc > 1)
		media_home = argv[1];
	else
		media_home = root.get<string>("rplay.media_home", media_home);

	if (argc > 2)
		port = lexical_cast<unsigned short>(argv[2]);
	else
		port = root.get<unsigned short>("rplay.port", port);

	log_file = root.get<string>("rplay.log_file", string{});
}
