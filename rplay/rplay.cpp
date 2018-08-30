// remote player implementation
#include <string>
#include <iostream>
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

	cout << software_name() << " " << software_version() << " (" << software_build() << ")\n";
	cout << "listenning on tcp://*:" << conf.port << "\n";
	cout << "media-home: " << conf.media_home << "\n";

	library lib{conf.media_home};

	player play;
	zmq_interface iface{conf.port, &lib, &play};

	iface.run();
	play.start();
	play.play();  // play queue

	iface.join();

	return 0;
}
