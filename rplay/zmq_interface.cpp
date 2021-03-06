#include <algorithm>
#include <functional>
#include <string>
#include <iostream>
#include "zmqu/send.hpp"
#include "zmqu/json.hpp"
#include "rplib/time.hpp"
#include "rplib/log.hpp"
#include "zmq_interface.hpp"
#include "version.hpp"

using std::abs;
using std::string;
using std::to_string;
using std::vector;
using std::lock_guard;
using std::mutex;
using std::cout;
using std::cerr;

static void vector_put(jtree & root, string const & key, vector<string> const & v);
static string unknown_command_answer(string const & cmd);
static string not_yet_implemented_answer(string const & cmd);

static size_t invalid_idx = (size_t)-1;

namespace zmqu {

//template <>
//size_t send_one<fs::path>(zmq::socket_t & sock, fs::path const & p, int flags)
//{
//	return send_one(sock, p.string(), flags);
//}

}  // zmqu

void vector_put(jtree & root, string const & key, vector<string> const & v)
{
	jtree arr;
	for (string const & elem : v)
	{
		jtree local;
		local.put("", elem);
		arr.push_back(make_pair("", local));
	}
	root.add_child(key, arr);
}

zmq_interface::zmq_interface(unsigned short port, library * lib, player * play)
	: _port{port}
	, _lib{lib}
	, _play{play}
	, _position_change_count{0}
	, _media_idx{0}
	, _position{0}
	, _duration{0}
	, _playlist_id{0}
{
	_tp = hires_clock::now() + std::chrono::seconds{1};

	_event_handler_id.push_back(
		_play->play_signal.add(std::bind(&zmq_interface::on_play, this, std::placeholders::_1, std::placeholders::_2)));

	_event_handler_id.push_back(
		_play->position_change_signal.add(std::bind(&zmq_interface::on_position_change, this, std::placeholders::_1, std::placeholders::_2)));

	_event_handler_id.push_back(
		_play->playlist_change_signal.add(std::bind(&zmq_interface::on_playlist_change, this, std::placeholders::_1, std::placeholders::_2)));
}

zmq_interface::~zmq_interface()
{
	_play->play_signal.remove_id(_event_handler_id[0]);
	_play->position_change_signal.remove_id(_event_handler_id[1]);
	_play->playlist_change_signal.remove_id(_event_handler_id[2]);
}

void zmq_interface::idle()
{
	auto now = hires_clock::now();
	if (now > _tp)
	{
		send_server_alive();
		_tp = now + std::chrono::seconds{1};
	}

	std::this_thread::sleep_for(std::chrono::milliseconds{10});
}

void zmq_interface::run()
{
	bind(_port, _port+1, _port+2);
	_th = std::thread{&zmqu::clone_server::start, this};
}

void zmq_interface::stop()
{
	quit();
	if (_th.joinable())
		_th.join();
}

void zmq_interface::join()
{
	_th.join();
}

void zmq_interface::on_play(std::string media, size_t media_idx)
{
	_position_change_count = 0;

	lock_guard<mutex> lock{_media_info_locker};
	_media = media;
	_media_idx = media_idx;
}

void zmq_interface::send_library_update() const
{
	jtree msg;
	msg.put("cmd", "library_update");
	LOG(trace) << "RPLAYC << library_update";
	publish(to_string(msg));
}

void zmq_interface::send_play_progress() const
{
	jtree msg;
	msg.put("cmd", "play_progress");
	msg.put<long>("position", static_cast<long>(_position));
	msg.put<long>("duration", static_cast<long>(_duration));
	msg.put<size_t>("playlist_id", _playlist_id);
	msg.put<size_t>("media_idx", _media_idx);

	int playback_state = _play->paused() ? 2 : 1;
	msg.put<int>("playback_state", playback_state);

	int modes = (_play->bed_time() << 1)|_play->shuffle();
	msg.put<int>("mode", modes);

	LOG(trace) << "RPLAYC << play_progress(position=" << _position
		<< ", duration=" << _duration << ", playlist_id=" << _playlist_id
		<< ", media_idx=" << _media_idx << ", playback_state="
		<< playback_state << ", mode=" << modes << ")";

	publish(to_string(msg));
}

void zmq_interface::send_playlist_content()
{
	jtree msg;
	msg.put("cmd", "playlist_content");
	msg.put<size_t>("id", _playlist_id);
	vector_put(msg, "items", _playlist);

	LOG(trace) << "RPLAYC << playlist_content(id=" << _playlist_id << ", "
		<< _playlist.size() << " items)";

	publish(to_string(msg));
}

void zmq_interface::send_server_alive()
{
	static unsigned alive_counter = 1;

	jtree alive;
	alive.put<string>("cmd", "alive");

	alive.put<size_t>("count", alive_counter);
	++alive_counter;

	string now = rpl::timestamp();
	alive.put<string>("time_stamp", now);

//	LOG(trace) << "RPLAYC << ping(count=" << alive_counter << ", time_stamp=" << now << ")";

	publish(to_string(alive));
}

void zmq_interface::send_volume()
{
	int value = (int)_volume.value();

	jtree news;
	news.put<string>("cmd", "volume");
	news.put<int>("value", value);
	publish(to_string(news));

	LOG(trace) << "RPLAYC << volume(value=" << value << ")";
}

void zmq_interface::on_position_change(int64_t position, int64_t duration)
{
	bool seek_send_progress = false;

	lock_guard<mutex> lock{_media_info_locker};
	seek_send_progress = abs(position - _position) > 1000000000;  // 1s (means seek)
	_position = position;
	_duration = duration;
	if (_media.empty())
		return;

	if ((_position_change_count++ % 100) == 0 || seek_send_progress)
		send_play_progress();
}

void zmq_interface::on_playlist_change(size_t playlist_id, vector<string> items)
{
	lock_guard<mutex> lock{_media_info_locker};
	_playlist_id = playlist_id;

	_playlist.clear();
	for (string const & item : items)
	{
		if (item.find("file://", 0, 7) == 0)
			_playlist.push_back(item.substr(7));
		else
			_playlist.push_back(item);
	}

	send_playlist_content();
}

string zmq_interface::on_question(string const & question)
{
	jtree json;
	to_json(question, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "list_media")
	{
		LOG(trace) << "RPLAYC >> list_media";

		vector<string> content = _lib->list_media();
		jtree a;
		a.put("cmd", "media_library");
		vector_put(a, "items", content);  // TODO: rename to items

		LOG(trace) << "RPLAYC << media_library(" << content.size() << " files)";

		return to_string(a);
	}
	else if (cmd == "identify")
	{
		LOG(trace) << "RPLAYC >> identify";

		jtree a;
		a.put("cmd", "server_desc");
		a.put("version", software_version());
		a.put("build", software_build());

		LOG(trace) << "RPLAYC << server_desc(" << software_version() << ", " << software_build() << ")";

		return to_string(a);
	}
	else  // unknown command
		return unknown_command_answer(cmd);
}

void zmq_interface::on_notify(string const & s)
{
	jtree json;
	to_json(s, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "play")
	{
		size_t pid = json.get<size_t>("playlist", 0);
		size_t idx = json.get<size_t>("idx", invalid_idx);

		LOG(trace) << "RPLAYC >> play(playlist_idx=" << pid << ", idx=" << idx << ")";

		if (pid == 0 || idx == invalid_idx)
		{
			LOG(warning) << "play(playlist_id=" << pid << ", idx=" << idx << ") ignored, reason invalid playlist ID or item index";
			return;
		}

		if (_play->is_latest_playlist(pid))
			_play->play(idx);
		else
			LOG(warning) << "play ignored, what: playlist outdated";
	}
	else if (cmd == "pause")
	{
		LOG(trace) << "RPLAYC >> pause";

		_play->pause();

		lock_guard<mutex> lock{_media_info_locker};
		send_play_progress();
	}
	else if (cmd == "stop")
	{
		LOG(trace) << "RPLAYC >> stop";

		_play->stop();

		lock_guard<mutex> lock{_media_info_locker};
		_media = "";
		_position = _duration = 0;

		send_play_progress();
	}
	else if (cmd == "seek")
	{
		int pos = json.get<int>("position", -1);
		string media = json.get<string>("media", "");

		LOG(trace) << "RPLAYC >> seek(pos=" << pos << ", '" << media << "')";

		_play->seek(int64_t{pos} * 1000000000);
	}
	else if (cmd == "set_volume")
	{
		int value = json.get<int>("value", -1);

		LOG(trace) << "RPLAYC >> set_volume(value=" << value << ")";

		if (value >= 0 && value <= 100)
		{
			_volume.value(value);
			send_volume();
		}
		else
			LOG(warning) << "set_volume command ignored, value=" << value << " not in range.";
	}
	else if (cmd == "playlist_add")
	{
		vector<string> media;
		for (jtree::value_type & obj : json.get_child("items"))
			media.push_back(obj.second.data());

		LOG(trace) << "RPLAYC >> playlist_add(media='" << media.size() << " items')";

		if (!media.empty())
			_play->add(media);
	}
	else if (cmd == "playlist_remove")
	{
		size_t pid = json.get<size_t>("playlist", 0);

		vector<size_t> items;
		for (jtree::value_type & obj : json.get_child("items"))
			items.push_back(obj.second.get_value<size_t>());

		LOG(trace) << "RPLAYC >> playlist_remove(playlist=" << pid << ", items='" << items.size() << " items')";

		if (pid == 0 || items.empty())
		{
			LOG(warning) << "play(playlist_id=" << pid << ", items=[<empty>]) ignored, reason invalid playlist ID or item index";
			return;
		}

		if (_play->is_latest_playlist(pid))
			_play->remove(items);
		else
			LOG(warning) << "playlist outdated";

	}
	else if (cmd == "playlist_move")
	{
		size_t pid = json.get<size_t>("playlist", 0);
		size_t from_idx = json.get<size_t>("from", -1u);
		size_t to_idx = json.get<size_t>("to", -1u);

		LOG(trace) << "playlist_move(playlist_id=" << pid << ", from= "
			<< from_idx << ", to=" << to_idx << ")";

		if (pid == 0 || from_idx == -1u || to_idx == -1u)
		{
			LOG(warning) << "playlist_move(playlist_id=" << pid << ", from= "
				<< from_idx << ", to=" << to_idx << ") ignored, reason: invalid playlist ID or indices";
			return;
		}

		if (!_play->move(pid, from_idx, to_idx))
			LOG(warning) << "playlist outdated";
	}
	else if (cmd == "playlist_shuffle")
	{
		bool shuffle = json.get<bool>("shuffle", false);
		_play->shuffle(shuffle);
		LOG(trace) << "RPLAYC >> playlist_shuffle(" << shuffle << ")";

		lock_guard<mutex> lock{_media_info_locker};
		send_play_progress();
	}
	else if (cmd == "client_ready")
	{
		send_volume();

		{
			lock_guard<mutex> lock{_media_info_locker};

			if (_playlist.size() > 0)
				send_playlist_content();

			if (_play->playing())
				send_play_progress();
		}

		LOG(trace) << "RPLAYC >> client_ready";
	}
	else if (cmd == "bed_time")
	{
		bool value = json.get<bool>("value");
		_play->bed_time(value);
		LOG(trace) << "RPLAYC >> bed_time(" << value << ")";

		lock_guard<mutex> lock{_media_info_locker};
		send_play_progress();
	}
	else
		LOG(warning) << "unknown command (" << s << ")";
}

void zmq_interface::on_accepted(socket_id sid, std::string const & addr)
{
	zmqu::clone_server::on_accepted(sid, addr);

	if (sid != socket_id::PUBLISHER)
		return;

	LOG(trace) << "client '" << addr << "' connected";
}

string unknown_command_answer(string const & cmd)
{
	jtree json;
	json.put("cmd", "error");
	json.put("what", "unknown '" + cmd + "' command");
	return to_string(json);
}

string not_yet_implemented_answer(string const & cmd)
{
	jtree json;
	json.put("cmd", "error");
	json.put("what", "'" + cmd + "' command not yet implemented");
	return to_string(json);
}
