// play locally stored file file
#include <string>
#include <iostream>
#include "player.hpp"
#include "library.hpp"
#include "interface.hpp"
#include "config.hpp"

using std::string;
using std::cout;

int main(int argc, char * argv[])
{
	config conf{argc, argv};

	cout << "remoteplayer\n";
	cout << "listenning on tcp://*:" << conf.port << "\n";
	cout << "media-home: " << conf.media_home << "\n";

	player_init(&argc, &argv);

	library lib{conf.media_home};
	player play;
	play.init();

	interface iface{conf.port, &lib, &play};
	iface.run();

	iface.join();

	return 0;
}
