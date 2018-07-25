#pragma once
#include <chrono>
#include <zmqu/clone_server.hpp>
#include "library.hpp"
#include "player.hpp"

//! zmq based interface implementation
class interface
	: public zmqu::clone_server
{
public:
	using hires_clock = std::chrono::high_resolution_clock;

	interface(unsigned short port, library * lib, player * play);
	void run();
	void stop();
	void join();

private:
	void idle() override;
	std::string on_question(std::string const & question) override;
	void on_notify(std::string const & s) override;

	unsigned short _port;
	library * _lib;
	player * _play;
	std::thread _t;
	hires_clock::time_point _tp;
};
