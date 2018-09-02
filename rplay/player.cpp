#include <algorithm>
#include <functional>
#include "fs.hpp"
#include "player.hpp"

using std::find;
using std::bind;
using std::mutex;
using std::lock_guard;
using std::unique_lock;
using std::vector;
using std::string;
using namespace std::placeholders;

player::player()
	: _play_flag{0}
	, _playlist_id{1}
{}

void player::start()
{
	_th = std::thread{&player::loop, this};
}

void player::queue(string const & media)
{
	_items.add("file://" + media);
	++_playlist_id;
	playlist_change_signal.call(_playlist_id, _items.items());
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

bool player::playing() const
{
	return _play_flag;
}

playlist const & player::media_playlist() const
{
	return _items;
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
			string media = _items.wait_next();
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
