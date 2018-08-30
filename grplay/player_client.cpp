#include <iostream>
#include <zmqu/json.hpp>
#include "player_client.hpp"
#include "log.hpp"

using std::vector;
using std::unique_lock;
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

		assert(duration > 0);

		LOG(info) << "RPLAY >> play_progress(media='" << media << "', posiiton=" << position
			<< ", duration=" << duration;

		for (auto * l : listeners())
			l->on_play_progress(media, position, duration);
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

	unique_lock<mutex> lock{_mtx};
	_connected.wait(lock);  // wait for media_library content
}

void player_client::on_answer(std::string const & answer)
{
	jtree json;
	to_json(answer, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "media_library")
	{
		_media_library.clear();
		for (jtree::value_type & obj : json.get_child("content"))
			_media_library.push_back(fs::path{obj.second.data()});

		// \debug
//		string content{"{\"content\":[\n"};
//		for (jtree::value_type & obj : json.get_child("content"))
//			content += "  \"" + obj.second.data() + "\",\n";
//		content += "  \"\"\n]}";
//		save_to_file("content.json", content);
		// \enddebug

		_connected.notify_all();
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

vector<fs::path> const & player_client::list_media() const
{
	return _media_library;
}

void player_client::play(fs::path const & media)
{
	jtree req;
	req.put("cmd", "play_media");
	req.put("content", media.string());
	notify(to_string(req));

	LOG(trace) << "RPLAY << play_media(content='" << media.string() << "')";
}

void player_client::stop()
{
	jtree req;
	req.put("cmd", "stop");
	notify(to_string(req));

	LOG(trace) << "RPLAY << stop";
}
