// play locally stored file file
#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include <unistd.h>

#include "fs.hpp"
#include "player.hpp"
#include "library.hpp"

using std::string;
using std::vector;
using std::cout;
using std::cerr;

int main(int argc, char * argv[])
{
	player_init(&argc, &argv);

	library lib{"/home/adam/Music"};
	vector<fs::path> content = lib.list_media();

	cout << "media library:\n";
	for (size_t i = 0; i < content.size(); ++i)
		cout << "  #" << i << ". " << content[i] << "\n";

	fs::path const & media = content[10];

	player p;
	p.init();
	p.queue(media);
	p.join();

	return 0;
}
