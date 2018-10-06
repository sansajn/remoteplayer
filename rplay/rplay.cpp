// remote player implementation
#include <string>
#include <iostream>
#include "rplib/log.hpp"
#include "player.hpp"
#include "zmq_interface.hpp"
#include "library.hpp"
#include "config.hpp"
#include "version.hpp"

using std::string;
using std::cout;

int main(int argc, char * argv[])
{
	config conf{argc, argv};

	if (!conf.log_file.empty())
	{
		rpl::log_to_file(conf.log_file);

		LOG(info) << software_name() << " " << software_version() << " (" << software_build() << ")";
		LOG(info) << "listenning on tcp://*:" << conf.port;
		LOG(info) << "media-home: " << conf.media_home;
		LOG(info) << "log-file: " << conf.log_file;
	}
	else
		rpl::log_to_console();

	cout << software_name() << " " << software_version() << " (" << software_build() << ")\n";
	cout << "listenning on tcp://*:" << conf.port << "\n";
	cout << "media-home: " << conf.media_home << "\n";
	if (!conf.log_file.empty())
		cout << "log-file: " << conf.log_file << "\n";

	library lib{conf.media_home};

	player play;
	zmq_interface iface{conf.port, &lib, &play};

	iface.run();
	play.start();

	iface.join();

	return 0;
}
