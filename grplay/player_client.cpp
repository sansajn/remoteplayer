#include <zmqu/json.hpp>
#include "rplib/time.hpp"
#include "rplib/log.hpp"
#include "player_client.hpp"

using std::vector;
using std::unique_lock;
using std::lock_guard;
using std::mutex;
using std::string;


void rplay_client::send_ready() const
{
	jtree req;
	req.put<string>("cmd", "client_ready");
	notify(to_string(req));

	LOG(trace) << "RPLAY << client_ready";
}

void rplay_client::on_news(std::string const & news)
{
	jtree json;
	to_json(news, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "play_progress")
	{
		long position = json.get<long>("position", 0L);
		long duration = json.get<long>("duration", 0L);
		size_t playlist_id = json.get<size_t>("playlist_id", 0);
		size_t media_idx = json.get<size_t>("media_idx", 0);
		int playback_state = json.get<int>("playback_state", 0);
		int mode = json.get<int>("mode", 0);

		assert(playback_state > 0);

		LOG(trace) << "RPLAY >> play_progress(position=" << position
			<< ", duration=" << duration << ", playlist_id=" << playlist_id
			<< ", media_idx=" << media_idx << ", playback_state=" << playback_state
			<< ", mode=" << mode << ")";

		if (playlist_id > 0)
		{
			playback_state_e state = (playback_state_e)playback_state;
			for (auto * l : listeners())
				l->on_play_progress(position, duration, playlist_id, media_idx, state, mode);
		}
		else
			for (auto * l : listeners())
				l->on_stop();
	}
	else if (cmd == "playlist_content")
	{
		size_t const id = json.get<size_t>("id", 0);
		assert(id != 0 && "invalid playlist ID");

		vector<string> media_playlist_copy;

		{
			lock_guard<mutex> lock{_rplay_data_locker};

			_media_playlist.clear();
			for (jtree::value_type & obj : json.get_child("items"))
				_media_playlist.push_back(obj.second.data());

			media_playlist_copy = _media_playlist;
		}

		LOG(trace) << "RPLAY >> playlist_content(id=" << id << ", items='" << _media_playlist.size() << " items')";

		for (auto * l : listeners())
			l->on_playlist_change(id, media_playlist_copy);
	}
	else if (cmd == "alive")
	{
		size_t count = json.get<size_t>("count", 0);
		string time_stamp = json.get<string>("time_stamp", "");

		rpl::ptime t0 = rpl::to_ptime(time_stamp);
		rpl::time_duration dt = rpl::now() - t0;

//		LOG(trace) << "RPLAY >> alive(" << count << ") in " << dt.total_microseconds()/1000.0 << "ms";
	}
	else if (cmd == "volume")
	{
		int value = json.get<int>("value", -1);
		LOG(trace) << "RPLAY >> volume(value=" << value << ")";

		for (auto * l : listeners())
			l->on_volume(value);
	}
	else
	{
		if (!cmd.empty())
			LOG(warning) << "unknown command '" << cmd << "'";
		else
			LOG(warning) << "unknown command '" << news << "'";
	}
}

rplay_client::~rplay_client()
{
	quit();
	if (_t.joinable())
		_t.join();
}

rplay_client::rplay_client()
	: _connected_flag{false}, _connected{false, false, false}
{}

void rplay_client::connect(std::string const & host, unsigned short port)
{
	zmqu::clone_client::connect(host, port, port+1, port+2);
	_t = std::thread{&rplay_client::loop, this};

	std::this_thread::sleep_for(std::chrono::milliseconds{200});  // wait for thread
}

void rplay_client::on_answer(std::string const & answer)
{
	jtree json;
	to_json(answer, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "media_library")
	{
		vector<string> media_library_copy;

		{
			lock_guard<mutex> lock{_rplay_data_locker};
			_media_library.clear();
			for (jtree::value_type & obj : json.get_child("items"))
				_media_library.push_back(obj.second.data());

			media_library_copy = _media_library;
		}

		LOG(trace) << "RPLAY >> media_library('" << media_library_copy.size() << " items')";

		for (auto * l : listeners())
			l->on_list_media(media_library_copy);
	}
	else if (cmd == "server_desc")
	{
		string version = json.get("version", "0");
		string build = json.get("build", "0");
		LOG(trace) << "RPLAY >> server_desc(version='" << version << "', build='" << build << "')";
	}
	else
		LOG(warning) << "unknown answer: " << answer;
}

void rplay_client::idle()
{
	bool connected = (_connected[0] && _connected[1] && _connected[2]);

	if (connected && !_connected_flag)
	{
		send_ready();
		ask_identify();
		ask_list_media();
	}

	_connected_flag = connected;

	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}


void rplay_client::on_connected(socket_id sid, std::string const & addr)
{
	_connected[sid] = true;
}

void rplay_client::on_closed(socket_id sid, std::string const & addr)
{
	_connected[sid] = false;
}

vector<string> rplay_client::list_library() const
{
	lock_guard<mutex> lock{_rplay_data_locker};
	return _media_library;
}

vector<string> rplay_client::list_playlist() const
{
	lock_guard<mutex> lock{_rplay_data_locker};
	return _media_playlist;
}

void rplay_client::play(size_t playlist_id, size_t playlist_idx)
{
	jtree req;
	req.put("cmd", "play");
	req.put<size_t>("playlist", playlist_id);
	req.put<size_t>("idx", playlist_idx);
	notify(to_string(req));

	LOG(trace) << "RPLAY << play(playlist=" << playlist_id << ", idx=" << playlist_idx << ")";
}

void rplay_client::pause()
{
	jtree req;
	req.put("cmd", "pause");
	notify(to_string(req));

	LOG(trace) << "RPLAY << pause";
}

void rplay_client::stop()
{
	jtree req;
	req.put("cmd", "stop");
	notify(to_string(req));

	LOG(trace) << "RPLAY << stop";
}

void rplay_client::seek(long pos, fs::path const & media)
{
	jtree req;
	req.put("cmd", "seek");
	req.put<int>("position", int(pos / 1000000000));
	req.put("media", media.string());
	notify(to_string(req));

	LOG(trace) << "RPLAY << seek(pos=" << pos / 1000000000 << "s, media=" << media << ")";
}

void rplay_client::volume(int val)
{
	if (val < 0 || val > 100)
		return;  // out of range

	jtree req;
	req.put("cmd", "set_volume");
	req.put<int>("value", val);
	notify(to_string(req));

	LOG(trace) << "RPLAY << set_volume(value=" << val << ")";
}

void rplay_client::playlist_add(vector<fs::path> const & media)
{
	jtree req;
	req.put("cmd", "playlist_add");

	vector<string> items;
	for (fs::path const & item : media)
		items.push_back(item.string());
	vector_put(items, "items", req);

	notify(to_string(req));

	LOG(trace) << "RPLAY << playlist_add(items='" << media.size() << " items')";
}

void rplay_client::playlist_remove(size_t playlist_id, vector<size_t> const & items)
{
	jtree req;
	req.put("cmd", "playlist_remove");
	req.put("playlist", playlist_id);
	vector_put(items, "items", req);

	notify(to_string(req));

	LOG(trace) << "RPLAY << playlist_remove(items='" << items.size() << " items')";
}

void rplay_client::playlist_move_item(size_t playlist_id, size_t from_idx, size_t to_idx)
{
	jtree req;
	req.put("cmd", "playlist_move");
	req.put("playlist", playlist_id);
	req.put("from", from_idx);
	req.put("to", to_idx);

	notify(to_string(req));

	LOG(trace) << "RPLAY << playlist_move(from=" << from_idx << ", to=" << to_idx << ")";
}

void rplay_client::playlist_shuffle(bool shuffle) const
{
	jtree req;
	req.put("cmd", "playlist_shuffle");
	req.put<bool>("shuffle", shuffle);

	notify(to_string(req));

	LOG(trace) << "RPLAY << playlist_shuffle(" << shuffle << ")";
}

void rplay_client::bed_time(bool value) const
{
	jtree req;
	req.put("cmd", "bed_time");
	req.put<bool>("value", value);

	notify(to_string(req));

	LOG(trace) << "RPLAY << bed_time(" << value << ")";
}

void rplay_client::ask_identify()
{
	jtree req;
	req.put("cmd", "identify");

	ask(to_string(req));

	LOG(trace) << "RPLAY << identify";
}

void rplay_client::ask_list_media()
{
	jtree req;
	req.put("cmd", "list_media");

	ask(to_string(req));

	LOG(trace) << "RPLAY << list_media";
}

void rplay_client::loop()
{
	assert(!_connected[0] && !_connected[1] && !_connected[2]);
	start();
}
