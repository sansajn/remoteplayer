#pragma once
#include <atomic>
#include <thread>
#include <string>
#include "event.hpp"
#include "concurrent_queue.hpp"
#include "gst_audio_player.hpp"

//! highest level player abstraction with playlist support
class player
{
public:
	void start();
	void queue(std::string const & media);
	void play();
	void stop();
	bool playing() const;
	void quit();
	void join();

	// signals
	event<std::string> play_signal;
	event<int64_t, int64_t> position_change_signal;  //!< (position, duration) in ns

private:
	void loop();

	// \note called from gst_audio_player thread
	void item_done_cb();
	void item_progress_cb(int64_t position, int64_t duration);

	gst_audio_player _p;
	concurrent_queue<std::string> _items;  // list of media uris
	std::atomic_bool _play_flag, _quit_flag;
	std::thread _th;
};
