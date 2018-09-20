#include <boost/lexical_cast.hpp>
#include "config.hpp"
#include "json.hpp"
#include "log.hpp"
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
	jtree root;
	try {
		root = read_json_file(DEFAULT_CONFIG_FILE_NAME);
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
