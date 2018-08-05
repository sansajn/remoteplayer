// experimental player
#include <string>
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <gst/gst.h>
#include "../gst_audio_player.hpp"
#include "../helpers.hpp"

using std::string;
using std::cerr;
using std::cout;


class foo_player : public gst_audio_player
{
private:
	void on_position_changed(string media, long position) override;
};

void foo_player::on_position_changed(string media, long position)
{
	if (position > 1e10)  // ~10s
		stop();

	gst_audio_player::on_position_changed(media, position);
}

using async_player = gst_audio_player;

int main(int argc, char * argv[])
{
	string const input = (argc > 1) ? argv[1] : "test.opus";
	gst_init(&argc, &argv);
	foo_player p;
	p.play("file://" + pwd() + "/" + input);
	return 0;
}
