#pragma once
#include <thread>
#include <condition_variable>
#include <vector>
#include <zmqu/clone_client.hpp>
#include "fs.hpp"

struct player_client_listener
{
	virtual void on_play_progress(std::string const & media, long position, long duration) {}
};

// TODO: implement notify function
template <typename Listener>
class observable_with
{
public:
	void register_listener(Listener * l);  // TODO: find something better then register
	void forget_listener(Listener * l);
	std::vector<Listener *> & listeners();
	std::vector<Listener *> const & listeners() const;

private:
	std::vector<Listener *> _listeners;
};

template <typename Listener>
void observable_with<Listener>::register_listener(Listener * l)
{
	if (find(_listeners.begin(), _listeners.end(), l) == _listeners.end())
		_listeners.push_back(l);
}

template <typename Listener>
void observable_with<Listener>::forget_listener(Listener * l)
{
	remove(_listeners.begin(), _listeners.end(), l);
}

template <typename Listener>
std::vector<Listener *> & observable_with<Listener>::listeners()
{
	return _listeners;
}

template <typename Listener>
std::vector<Listener *> const & observable_with<Listener>::listeners() const
{
	return _listeners;
}



class player_client
	: public zmqu::clone_client
	, public observable_with<player_client_listener>
{
public:
	void connect(std::string const & host, unsigned short port);
	void disconnect();
	std::vector<fs::path> const & list_media() const;
	void play(fs::path const & media);
	void stop();

private:
	void on_news(std::string const & news) override;
	void on_answer(std::string const & answer) override;

	std::thread _t;
	std::vector<fs::path> _media_library;
	std::condition_variable _connected;
	std::mutex _mtx;
};
