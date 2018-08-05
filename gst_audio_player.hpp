#pragma once
#include <mutex>
#include <string>
#include <gst/gst.h>
#include "helpers.hpp"

class playbin // non_copyable
{
public:
	playbin();
	~playbin();
	bool play(std::string const & uri);
	bool stop();
	long position() const;  //!< in ns
	long duration() const;  //!< in ns
	GstElement * to_gst();

private:
	GstElement * _playbin;
};


/*! thread safe audio player based on GStreamer library
TODO: deal with errors and logging */
class gst_audio_player
{
public:
	gst_audio_player();
	~gst_audio_player();
	void play(std::string const & uri);
	void stop();
	std::string const & media() const;
	long position() const;
	long duration() const;
	bool playing() const;

	// events
	virtual void on_play(std::string media, long duration);
	virtual void on_eos(std::string media);
	virtual void on_position_changed(std::string media, long position);  //!< \param position in ns

private:
	void loop();
	bool handle_message(GstMessage * msg);

	// pipelane state
	long _position;  //!< in ns
	long _duration;  //!< in ns
	bool _playing;
	std::string _media;  //!< curently played media
	mutable std::mutex _pline_locker;

	watch_alert _position_changed_alert;
	playbin _playbin;
};
