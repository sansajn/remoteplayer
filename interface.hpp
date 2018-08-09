#pragma once
#include <chrono>
#include <zmqu/clone_server.hpp>
#include "library.hpp"
#include "player.hpp"
#include "helpers.hpp"

/*! zmq based interface implementation
TODO: define durations as sizeed integer (int64_t) */
class interface
	: public zmqu::clone_server
	, public player_listener
{
public:
	using hires_clock = std::chrono::high_resolution_clock;

	interface(unsigned short port, library * lib, player * play);
	~interface();
	void run();
	void stop();
	void join();

//	void on_queue_changed(player_listener::queue_operation op, fs::path item) override;
	void on_play(fs::path item, long duration) override;
	void on_position_changed(fs::path item, long position) override;

private:
	void idle() override;
	std::string on_question(std::string const & question) override;
	void on_notify(std::string const & s) override;

	unsigned short _port;
	library * _lib;
	player * _play;
	std::thread _t;
	hires_clock::time_point _tp;
	long _cur_audio_duration;
	watch_alert _pos_change_alert;
};
