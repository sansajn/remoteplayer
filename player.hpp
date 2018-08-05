#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include "fs.hpp"
#include "concurrent_queue.hpp"
#include "gst_audio_player.hpp"

struct player_listener
{
	enum class queue_operation {push, pop};

	virtual void on_play(fs::path item, int duration) = 0;
	virtual void on_position_changed(fs::path item, long position) {}
};

class observable_audio_player_adapter : public gst_audio_player
{
public:
	void register_listenner(player_listener * l);
	void forget_listenner(player_listener * l);

private:
	void on_play(std::string media, long duration) override;
	void on_position_changed(std::string media, long position) override;
	// on_eos

	std::vector<player_listener *> _listeners;
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

	concurrent_queue<fs::path> _items;
	observable_audio_player_adapter _ap;

	std::thread _t;
	std::vector<player_listener *> _listeners;
};

//! player module initialization
void player_init(int * argc, char ** argv[]);
