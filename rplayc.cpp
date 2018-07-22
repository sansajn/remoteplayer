// remote player client
#include <vector>
#include <string>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <zmqu/clone_client.hpp>
#include <zmqu/json.hpp>
#include "fs.hpp"

using std::vector;
using std::string;
using std::cin;
using std::cout;
using std::condition_variable;
using std::mutex;
using std::unique_lock;

class player
	: public zmqu::clone_client
{
public:
	void connect(std::string const & host);
	void disconnect();
	vector<fs::path> const & list_media();
	void play(fs::path const & media);

private:
	void on_answer(std::string const & answer) override;

	std::thread _t;
	std::vector<fs::path> _media_library;
	condition_variable _connected;
	mutex _mtx;
};

void player::connect(std::string const & host)
{
	zmqu::clone_client::connect(host, 13333, 13334, 13335);
	_t = std::thread{&zmqu::clone_client::start, this};

	// ask for media_library
	jtree req;
	req.put("cmd", "list_media");
	ask(to_string(req));

	unique_lock<mutex> lock{_mtx};
	_connected.wait(lock);  // wait for media_library content
}

void player::on_answer(std::string const & answer)
{
	jtree json;
	to_json(answer, json);
	string const cmd = json.get("cmd", string{});

	if (cmd == "media_library")
	{
		_media_library.clear();
		for (jtree::value_type & obj : json.get_child("content"))
			_media_library.push_back(fs::path{obj.second.data()});

		_connected.notify_all();
	}
	else
		cout << "unknown answer: " << answer << std::endl;
}

vector<fs::path> const & player::list_media()
{
	return _media_library;
}

void player::play(fs::path const & media)
{
	jtree req;
	req.put("cmd", "play_media");
	req.put("content", media.string());
	notify(to_string(req));
}

int main(int argc, char * argv[])
{
	string const host = (argc > 1) ? argv[1] : "localhost";

	player p;
	p.connect(host);

	vector<fs::path> content = p.list_media();
	cout << "media library:\n";
	for (size_t i = 0; i < content.size(); ++i)
		cout << "  #" << i << ". " << content[i] << "\n";

	size_t id;
	while (cin)
	{
		cout << "rplay> ";
		cin >> id;
		if (cin && (id < content.size()))
			p.play(content[id]);
	}

	return 0;
}
