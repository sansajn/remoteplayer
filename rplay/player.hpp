#pragma once
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>
#include "event.hpp"
#include "gst_audio_player.hpp"
#include "playlist.hpp"

//! highest level player abstraction with playlist support
class player
{
public:
	player();
	void start();
	void play(size_t idx);  //!< play from playlist
	void play();
	void pause();
	void seek(int64_t pos_in_ns);
	void stop();
	bool playing() const;
	bool paused() const;
	void add(std::vector<std::string> const & media);
	void remove(std::vector<size_t> const & items);
	bool move(size_t playlist_id, size_t from_idx, size_t to_idx);
	void shuffle(bool state);
	bool shuffle() const;
	playlist const & media_playlist() const;
	bool is_latest_playlist(size_t playlist_id);  // TODO: wrong design we need atomic play with index and playlist_id
	void clear_media_playlist();
	void quit();
	void join();

	// signals
	event<std::string, size_t> play_signal;  //!< (media, playlist_idx)
	event<int64_t, int64_t> position_change_signal;  //!< (position, duration) in ns
	event<size_t, std::vector<std::string>> playlist_change_signal;  //!< (id, playlist_items)

private:
	void loop();

	// \note called from gst_audio_player thread
	void item_done_cb();
	void item_progress_cb(int64_t position, int64_t duration);

	gst_audio_player _p;
	playlist _items;  // list of media URIs
	size_t _playlist_id;  // unique playlist identifier
	std::atomic_bool _play_flag, _pause_flag, _quit_flag;
	std::thread _th;
};
