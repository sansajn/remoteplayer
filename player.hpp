#pragma once
#include <thread>
#include "fs.hpp"
#include "concurrent_queue.hpp"

class player
{
public:
	player();
	void init();
	void queue(fs::path const & media);
	void quit();
	void join();

private:
	void loop();
	void play(fs::path const & media);

	concurrent_queue<fs::path> _items;
	std::thread _t;
};

//! player module initialization
void player_init(int * argc, char ** argv[]);
