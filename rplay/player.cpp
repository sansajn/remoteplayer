#include <algorithm>
#include <functional>
#include "rplib/log.hpp"
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
	, _quit_flag{false}
	, _bed_time{false}
{}

void player::start()
{
	_th = std::thread{&player::loop, this};
}

void player::play(size_t idx)
{
	_p.stop();
	if (_items.shuffle())
		_items.try_next();
	else
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

void player::remove(vector<size_t> const & items)
{
	_items.remove(items);

	++_playlist_id;
	playlist_change_signal.call(_playlist_id, _items.items());
}

bool player::move(size_t playlist_id, size_t from_idx, size_t to_idx)
{
	if (playlist_id != _playlist_id)
		return false;

	_items.move(from_idx, to_idx);

	++_playlist_id;
	playlist_change_signal.call(_playlist_id, _items.items());

	return true;
}

void player::shuffle(bool state)
{
	_items.shuffle(state);
}

bool player::shuffle() const
{
	return _items.shuffle();
}

void player::bed_time(bool state)
{
	_bed_time = state;
}

bool player::bed_time() const
{
	return _bed_time;
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
//			string media;
//			if (!_items.try_next(media))
//			{
//				_play_flag = false;
//				LOG(debug) << "playback resumed";
//				continue;  // nothing new in playlist
//			}

			size_t idx = _items.current_item_idx();
			if (idx == playlist::npos)
			{
				_play_flag = false;
				LOG(debug) << "playback resumed";
				continue;  // nothing new in playlist
			}

			string media = _items.item(idx);

//			assert(_items.current_item_idx() > 0);  // one item ahead
			LOG(debug) << _items.current_item_idx();
			play_signal.call(media, _items.current_item_idx());
			_p.play(media, bind(&player::item_done_cb, this), bind(&player::item_progress_cb, this, _1, _2));
			_p.join();

			if (_bed_time)
				_play_flag = false;
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds{100});
	}

	LOG(trace) << "player::loop() done";
}

void player::item_done_cb()
{
	_items.try_next();
}

void player::item_progress_cb(int64_t position, int64_t duration)
{
	position_change_signal.call(position, duration);
}
