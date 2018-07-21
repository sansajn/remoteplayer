// play locally stored file file
#include <string>
#include <iostream>
#include <cassert>
#include <unistd.h>
#include "fs.hpp"
#include "player.hpp"

using std::string;
using std::cerr;

int main(int argc, char * argv[])
{
	if (argc < 2)
	{
		cerr << "input file missing, exit\n";
		return -1;
	}

	player_init(&argc, &argv);

	char buf[1024];
	getcwd(buf, sizeof(buf));
	fs::path media = fs::path{buf} / fs::path{argv[1]};

	player p;
	p.init();
	p.queue(media);
	p.join();

	return 0;
}
