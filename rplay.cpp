// play locally stored file file
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <zmqu/clone_server.hpp>
#include <zmqu/json.hpp>
#include <zmqu/send.hpp>
#include "player.hpp"
#include "library.hpp"

using std::string;
using std::vector;
using std::ostringstream;
using std::cout;
using std::cerr;


//! zmq based interface implementation
class interface
	: public zmqu::clone_server
{
public:
	interface(library * lib, player * play)
		: _lib{lib}, _play{play}
	{}

	void run();
	void stop();
	void join();

private:
	std::string on_question(std::string const & question) override;
	void on_notify(string const & s) override;

	library * _lib;
	player * _play;
	std::thread _t;
};

void interface::run()
{
	bind(13333, 13334, 13335);
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

string interface::on_question(string const & question)
{
	jtree json;
	to_json(question, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "list_media")
	{
		vector<fs::path> content = _lib->list_media();
		jtree a;
		a.put("cmd", "media_library");
		vector_put(a, "content", content);
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

int main(int argc, char * argv[])
{
	player_init(&argc, &argv);

	string const media_home = (argc > 1) ? argv[1] : "/home/adam/Music";

	library lib{media_home};
	player play;
	play.init();

	interface iface{&lib, &play};
	iface.run();

	iface.join();

	return 0;
}
