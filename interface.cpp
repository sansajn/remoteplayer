#include <string>
#include <iostream>
#include <zmqu/send.hpp>
#include <zmqu/json.hpp>
#include "interface.hpp"
#include "log.hpp"
#include "version.hpp"

using std::string;
using std::to_string;
using std::vector;
using std::cout;
using std::cerr;

static void vector_put(jtree & root, string const & key, vector<fs::path> const & v);
static string unknown_command_answer(string const & cmd);
static string not_yet_implemented_answer(string const & cmd);

namespace zmqu {

template <>
size_t send_one<fs::path>(zmq::socket_t & sock, fs::path const & p, int flags)
{
	return send_one(sock, p.string(), flags);
}

}  // zmqu

void vector_put(jtree & root, string const & key, vector<fs::path> const & v)
{
	jtree arr;
	for (fs::path const & p : v)
	{
		jtree local;
		local.put("", p.string());
		arr.push_back(make_pair("", local));
	}
	root.add_child(key, arr);
}

interface::interface(unsigned short port, library * lib, player * play)
	: _port{port}
	, _lib{lib}
	, _play{play}
	, _cur_audio_duration{0}
	, _pos_change_alert{10}
{
	_tp = hires_clock::now() + std::chrono::seconds{1};
	_play->register_listener(this);
}

interface::~interface()
{
	_play->forget_listener(this);
}

void interface::idle()
{
	static unsigned ping_counter = 1;

	auto now = hires_clock::now();
	if (now > _tp)
	{
		_tp = now + std::chrono::seconds{1};

		jtree ping;
		ping.put("ping", to_string(ping_counter));
		++ping_counter;

		publish(to_string(ping));
	}
}

void interface::run()
{
	bind(_port, _port+1, _port+2);
	_t = std::thread{&zmqu::clone_server::start, this};
}

void interface::stop()
{
	quit();
	if (_t.joinable())
		_t.join();
}

void interface::join()
{
	_t.join();
}

//void interface::on_queue_changed(player_listener::queue_operation op, fs::path item)
//{
//	jtree news;
//	news.put("cmd", "queue_changed");
//	news.put("operation", (op == player_listener::queue_operation::push) ? "push" : "pop");
//	news.put("media", item.string());

//	publish(to_string(news));

//	LOG(trace) << "RPLAYC << queue_changed(op="
//		<< ((op == player_listener::queue_operation::push) ? "push" : "pop")
//		<< ", media=" << item << ")";
//}

void interface::on_play(fs::path item, long duration)
{
	_cur_audio_duration = duration;

// TODO: remove (play command removed from spec)
//	jtree news;
//	news.put("cmd", "play");
//	news.put("media", item.string());
//	news.put<long>("duration", duration);

//	publish(to_string(news));

//	LOG(trace) << "RPLAYC << play(media=" << item.string() << ", duration="
//		<< duration << ")";
}



void interface::on_position_changed(fs::path item, long position)
{
	if (!_pos_change_alert.update(position))
		return;

	jtree news;
	news.put("cmd", "play_progress");
	news.put("media", item.string());
	news.put<long>("position", position);
	news.put<long>("duration", _cur_audio_duration);

	publish(to_string(news));

	LOG(trace) << "on_position_changed(item=" << item << ", position=" << position
		<< "ns, duration=" << _cur_audio_duration << "ns)";
}


string interface::on_question(string const & question)
{
	jtree json;
	to_json(question, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "list_media")
	{
		LOG(trace) << "RPLAYC >> list_media";

		vector<fs::path> content = _lib->list_media();
		jtree a;
		a.put("cmd", "media_library");
		vector_put(a, "content", content);

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

		LOG(trace) << "RPLAYC << server_desc(" << software_version() << ", " << software_build();

		return to_string(a);
	}
	else  // unknown command
		return unknown_command_answer(cmd);
}

void interface::on_notify(string const & s)
{
	jtree json;
	to_json(s, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "play_media")
	{
		string const content = json.get("content", string{});
		cout << content << " received" << std::endl;
		if (!content.empty())
			_play->queue(fs::path{content});
		else
			cerr << "warning: missing content (" << s << ")";
	}
	else
		cerr << "warning: unknown command (" << s << ")";
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
