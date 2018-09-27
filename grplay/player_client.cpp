#include <iostream>
#include <zmqu/json.hpp>
#include <rplib/time.hpp>
#include "player_client.hpp"
#include "log.hpp"

using std::vector;
using std::unique_lock;
using std::lock_guard;
using std::mutex;
using std::string;
using std::cout;

void player_client::on_news(std::string const & news)
{
	jtree json;
	to_json(news, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "play_progress")
	{
		string const media = json.get("media", string{});
		long position = json.get<long>("position", 0L);
		long duration = json.get<long>("duration", 0L);
		size_t playlist_idx = json.get<size_t>("playlist_idx", 0);

		assert(duration > 0);

		LOG(trace) << "RPLAY >> play_progress(media='" << media << "', posiiton=" << position
			<< ", duration=" << duration << ", playlist_idx=" << playlist_idx << ")";

		for (auto * l : listeners())
			l->on_play_progress(media, position, duration, playlist_idx);
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

		LOG(trace) << "RPLAY >> playlist_content(id=" << id << ", items=(" << _media_playlist.size() << " items)";

		for (auto * l : listeners())
			l->on_playlist_change(id, media_playlist_copy);
	}
	else if (cmd == "alive")
	{
		size_t count = json.get<size_t>("count", 0);
		string time_stamp = json.get<string>("time_stamp", "");

		rpl::ptime t0 = rpl::to_ptime(time_stamp);
		rpl::time_duration dt = rpl::now() - t0;

//		LOG(trace) << "RPLAY >> alive(" << count << ") in "
//			<< dt.total_microseconds()/1000.0 << "ms";
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

void player_client::connect(std::string const & host, unsigned short port)
{
	zmqu::clone_client::connect(host, port, port+1, port+2);
	_t = std::thread{&zmqu::clone_client::start, this};

	std::this_thread::sleep_for(std::chrono::milliseconds{200});  // wait for thread

	// ask for media_library
	jtree req;
	req.put("cmd", "list_media");
	ask(to_string(req));

	jtree req2;
	req2.put("cmd", "identify");
	ask(to_string(req2));
}

void player_client::on_answer(std::string const & answer)
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
			for (jtree::value_type & obj : json.get_child("content"))
				_media_library.push_back(obj.second.data());

			media_library_copy = _media_library;
		}

		for (auto * l : listeners())
			l->on_list_media(media_library_copy);

		// \debug
//		string content{"{\"content\":[\n"};
//		for (jtree::value_type & obj : json.get_child("content"))
//			content += "  \"" + obj.second.data() + "\",\n";
//		content += "  \"\"\n]}";
//		save_to_file("content.json", content);
		// \enddebug
	}
	else if (cmd == "server_desc")
	{
		string version = json.get("version", "0");
		string build = json.get("build", "0");
		LOG(debug) << "server=(" << version << ", " << build << ")";
	}
	else
		cout << "unknown answer: " << answer << std::endl;
}

vector<string> player_client::list_library() const
{
	lock_guard<mutex> lock{_rplay_data_locker};
	return _media_library;
}

vector<string> player_client::list_playlist() const
{
	lock_guard<mutex> lock{_rplay_data_locker};
	return _media_playlist;
}

void player_client::play(size_t playlist_id, size_t playlist_idx)
{
	jtree req;
	req.put("cmd", "play");
	req.put<size_t>("playlist", playlist_id);
	req.put<size_t>("idx", playlist_idx);
	notify(to_string(req));

	LOG(trace) << "RPLAY << play_media(playlist=" << playlist_id << ", idx=" << playlist_idx << ")";
}

void player_client::stop()
{
	jtree req;
	req.put("cmd", "stop");
	notify(to_string(req));

	LOG(trace) << "RPLAY << stop";
}

void player_client::seek(long pos, fs::path const & media)
{
	jtree req;
	req.put("cmd", "seek");
	req.put<int>("position", int(pos / 1000000000));
	req.put("media", media.string());
	notify(to_string(req));

	LOG(trace) << "RPLAY << seek(pos=" << pos / 1000000000 << "s, media=" << media << ")";
}

void player_client::volume(int val)
{
	if (val < 0 || val > 100)
		return;  // out of range

	jtree req;
	req.put("cmd", "set_volume");
	req.put<int>("value", val);
	notify(to_string(req));

	LOG(trace) << "RPLAY << set_volume(value=" << val << ")";
}

void player_client::playlist_add(fs::path const & media)
{
	jtree req;
	req.put("cmd", "playlist_add");
	req.put("media", media.string());
	notify(to_string(req));

	LOG(trace) << "RPLAY << playlist_add(" << media << ")";
}
