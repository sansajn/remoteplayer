#pragma once
#include <thread>
#include <vector>
#include "fs.hpp"
#include "concurrent_queue.hpp"

struct player_listener
{
	enum class queue_operation {push, pop};

	virtual void on_queue_changed(queue_operation op, fs::path item) = 0;
};

class player
{
public:
	player();
	void init();
	void queue(fs::path const & media);
	void quit();
	void join();

	void register_listener(player_listener * l);
	void forget_listener(player_listener * l);

private:
	void loop();
	void play(fs::path const & media);

	concurrent_queue<fs::path> _items;
	std::thread _t;
	std::vector<player_listener *> _listeners;
};

//! player module initialization
void player_init(int * argc, char ** argv[]);
