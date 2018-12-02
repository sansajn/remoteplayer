#pragma once
#include <thread>
#include <condition_variable>
#include <vector>
#include <zmqu/clone_client.hpp>
#include "fs.hpp"

enum class playback_state_e{invalid, playing, paused};

struct player_client_listener
{
	virtual void on_play_progress(std::string const & media, long position, long duration,
		size_t playlist_idx, playback_state_e playback_state) {}
	virtual void on_playlist_change(size_t playlist_id, std::vector<std::string> const & items) {}
	virtual void on_list_media(std::vector<std::string> const & items) {}
	virtual void on_volume(int val) {}
	virtual void on_stop() {}
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


class rplay_client
	: public zmqu::clone_client
	, public observable_with<player_client_listener>
{
public:
	rplay_client();
	~rplay_client();
	void connect(std::string const & host, unsigned short port);
	void disconnect();
	std::vector<std::string> list_library() const;
	std::vector<std::string> list_playlist() const;
	void play(size_t playlist_id, size_t playlist_idx);
	void pause();
	void stop();
	void seek(long pos, fs::path const & media);
	void volume(int val);  //!< val can be from [0, 100] range
	void playlist_add(std::vector<fs::path> const & media);
	void playlist_remove(size_t playlist_id, std::vector<size_t> const & items);
	void playlist_move_item(size_t playlist_id, size_t from_idx, size_t to_idx);
	void ask_identify();
	void ask_list_media();

private:
	void idle() override;
	void loop();
	void send_ready() const;
	void on_news(std::string const & news) override;
	void on_answer(std::string const & answer) override;
	void on_connected(socket_id sid, std::string const & addr) override;
	void on_closed(socket_id sid, std::string const & addr) override;

	std::vector<std::string> _media_library;
	std::vector<std::string> _media_playlist;

	std::thread _t;  //!< clone_client thread
	mutable std::mutex _rplay_data_locker;

	bool _connected_flag, _connected[3];
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
