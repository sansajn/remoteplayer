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
	: _playlist_id{1}
	, _play_flag{false}
	, _pause_flag{false}
{}

void player::start()
{
	_th = std::thread{&player::loop, this};
}

void player::play(size_t idx)
{
	_p.stop();
	_items.set_current_item(idx);
	play();
}

void player::play()
{
	_play_flag = true;
}

void player::pause()
{
	if (!_pause_flag)
	{
		_p.pause();
		_pause_flag = true;
	}
	else
	{
		_p.resume();
		_pause_flag = false;
	}
}

void player::seek(int64_t pos_in_ns)
{
	_p.seek(pos_in_ns);
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

bool player::paused() const
{
	return _pause_flag;
}

void player::add(vector<string> const & media)
{
	for (string const & item : media)
		_items.add("file://" + item);

	++_playlist_id;
	playlist_change_signal.call(_playlist_id, _items.items());
}

void player::remove(size_t playlist_idx)
{
	_items.remove(playlist_idx);
	++_playlist_id;
	playlist_change_signal.call(_playlist_id, _items.items());
}

playlist const & player::media_playlist() const
{
	return _items;
}

bool player::is_latest_playlist(size_t playlist_id)
{
	return playlist_id == _playlist_id;
}

void player::clear_media_playlist()
{
	_items.clear();
	++_playlist_id;
	playlist_change_signal.call(_playlist_id, _items.items());
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
			play_signal.call(media, _items.current_item_idx());
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
