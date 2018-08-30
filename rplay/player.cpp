#include <functional>
#include "fs.hpp"
#include "player.hpp"

using std::bind;
using std::string;
using namespace std::placeholders;

void player::start()
{
	_th = std::thread{&player::loop, this};
}

void player::queue(string const & media)
{
	_items.push("file://" + media);
}

void player::play()
{
	_play_flag = true;
}

void player::stop()
{
	_play_flag = false;
	_p.stop();
}

void player::quit()
{
	_quit_flag = true;
}

void player::join()
{
	_th.join();
}

void player::loop()
{
	while (!_quit_flag)
	{
		if (_play_flag)
		{
			string media;
			_items.wait_and_pop(media);
			play_signal.call(media);
			_p.play(media, bind(&player::item_done_cb, this), bind(&player::item_progress_cb, this, _1, _2));
			_p.join();
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds{200});
	}
}

void player::item_done_cb()
{}

void player::item_progress_cb(int64_t position, int64_t duration)
{
	position_change_signal.call(position, duration);
}
