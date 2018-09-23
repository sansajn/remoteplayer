#include <functional>
#include <string>
#include <iostream>
#include <zmqu/send.hpp>
#include <zmqu/json.hpp>
#include <rplib/time.hpp>
#include "zmq_interface.hpp"
#include "log.hpp"
#include "version.hpp"

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
	, _playlist_idx{0}
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

void zmq_interface::on_play(std::string media, size_t playlist_idx)
{
	cout << "playing '" << media << "' ..." << std::endl;

	_position_change_count = 0;

	lock_guard<mutex> lock{_media_info_locker};
	_media = media;
	_playlist_idx = playlist_idx;
}

void zmq_interface::send_play_progress()
{
	jtree msg;

	{
		lock_guard<mutex> lock{_media_info_locker};

		msg.put("cmd", "play_progress");
		msg.put("media", _media);
		msg.put<long>("position", (long)_position);
		msg.put<long>("duration", (long)_duration);
		msg.put<size_t>("playlist_idx", _playlist_idx);

		LOG(trace) << "RPLAYC << play_progress(media=" << _media << ", position="
			<< _position << ", duration=" << _duration << ", playlist_idx=" << _playlist_idx << ")";
	}

	publish(to_string(msg));
}

void zmq_interface::send_playlist_content()
{
	jtree msg;

	{
		lock_guard<mutex> lock{_media_info_locker};

		msg.put("cmd", "playlist_content");
		msg.put<size_t>("id", _playlist_id);
		vector_put(msg, "items", _playlist);

		LOG(trace) << "RPLAYC << playlist_content(id=" << _playlist_id << ", "
			<< _playlist.size() << " items)";
	}

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

	LOG(trace) << "RPLAYC << ping(count=" << alive_counter << ", time_stamp=" << now << ")";

	publish(to_string(alive));
}

void zmq_interface::on_position_change(int64_t position, int64_t duration)
{
//	cout << "progress=" << position << "/" << duration << std::endl;

	{
		lock_guard<mutex> lock{_media_info_locker};
		_position = position;
		_duration = duration;
	}

	assert(!_media.empty());

	if ((_position_change_count++ % 100) == 0)
		send_play_progress();
}

void zmq_interface::on_playlist_change(size_t playlist_id, vector<string> items)
{
	{
		lock_guard<mutex> lock{_media_info_locker};
		_playlist_id = playlist_id;
		_playlist = items;
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
		vector_put(a, "content", content);  // TODO: rename to items

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

	if (cmd == "play_media")
	{
		string const content = json.get("content", string{});
		if (!content.empty())
		{
			_play->queue(content);
			if (!_play->playing())
				_play->play();
		}

		LOG(trace) << "RPLAYC >> play_media(content='" << content << "')";
	}
	else if (cmd == "stop")
	{
		_play->stop();
		_play->clear_media_playlist();

		LOG(trace) << "RPLAYC >> stop()";
	}
	else if (cmd == "seek")
	{
		int pos = json.get<int>("position", -1);
		string media = json.get<string>("media", "");

		_play->seek(int64_t{pos} * 1000000000);

		LOG(trace) << "seek(pos=" << pos << ", '" << media << "')";
	}
	else
		LOG(warning) << "unknown command (" << s << ")";
}

void zmq_interface::on_accepted(socket_id sid, std::string const & addr)
{
	zmqu::clone_server::on_accepted(sid, addr);

	if (sid != socket_id::PUBLISHER)
		return;

	LOG(trace) << "client connected";

	if (!_playlist.empty())
		send_playlist_content();

	if (_play->playing())
		send_play_progress();
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
