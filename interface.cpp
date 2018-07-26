#include <string>
#include <iostream>
#include <zmqu/send.hpp>
#include <zmqu/json.hpp>
#include "interface.hpp"
#include "log.hpp"

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
{
	_tp = hires_clock::now() + std::chrono::seconds{1};
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