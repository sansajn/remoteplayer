#pragma once
#include <thread>
#include <condition_variable>
#include <vector>
#include <zmqu/clone_client.hpp>
#include "fs.hpp"
#include "rplib/observer.hpp"

enum class playback_state_e {invalid, playing, paused, stopped};

enum playlist_mode_e
{
	playlist_mode_shuffle = 0x1,
	playlist_mode_bed_time = 0x2
};

struct player_client_listener
{
	//! \param playlist_mode_list list of \c playlist_mode_e or-ed values
	virtual void on_play_progress(long position, long duration,	size_t playlist_id,
		size_t media_idx, playback_state_e playback_state, int playlist_mode_list) {}

	virtual void on_playlist_change(size_t playlist_id, std::vector<std::string> const & items) {}
	virtual void on_list_media(std::vector<std::string> const & items) {}
	virtual void on_volume(int val) {}
	virtual void on_stop() {}
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
	void playlist_shuffle(bool shuffle) const;
	void bed_time(bool value) const;
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
