#pragma once
#include <thread>
#include <condition_variable>
#include <vector>
#include <zmqu/clone_client.hpp>
#include "fs.hpp"

class player_client
	: public zmqu::clone_client
{
public:
	void connect(std::string const & host);
	void disconnect();
	std::vector<fs::path> const & list_media() const;
	void play(fs::path const & media);

private:
	void on_news(std::string const & news) override;
	void on_answer(std::string const & answer) override;

	std::thread _t;
	std::vector<fs::path> _media_library;
	std::condition_variable _connected;
	std::mutex _mtx;
};
