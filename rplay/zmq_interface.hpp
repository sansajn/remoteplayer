#pragma once
#include <thread>
#include <chrono>
#include <mutex>
#include <zmqu/clone_server.hpp>
#include "library.hpp"
#include "player.hpp"
#include "volume_control.hpp"

/*! zmq based interface implementation */
class zmq_interface
	: public zmqu::clone_server
{
public:
	using hires_clock = std::chrono::high_resolution_clock;

	zmq_interface(unsigned short port, library * lib, player * play);
	~zmq_interface();
	void run();  // TODO: rename to start
	void stop();
	void join();

private:
	void send_play_progress();
	void send_playlist_content();
	void send_server_alive();
	void send_volume();

	//! \note called from player thread
	void on_play(std::string media, size_t playlist_idx);
	void on_position_change(int64_t position, int64_t duration);
	void on_playlist_change(size_t playlist_id, std::vector<std::string> items);

	void idle() override;
	std::string on_question(std::string const & question) override;
	void on_notify(std::string const & s) override;
	void on_accepted(socket_id sid, std::string const & addr) override;

	unsigned short _port;
	library * _lib;
	player * _play;
	volume_control _volume;
	std::thread _th;
	hires_clock::time_point _tp;
	std::vector<size_t> _event_handler_id;

	int _position_change_count;

	// current media info
	std::string _media;
	size_t _playlist_idx;
	int64_t _position;
	int64_t _duration;
	size_t _playlist_id;
	std::vector<std::string> _playlist;
	std::mutex _media_info_locker;  // TODO: rename to player_info_locker
};
