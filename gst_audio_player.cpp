#include <cassert>
#include <iostream>
#include "gst_audio_player.hpp"

using std::lock_guard;
using std::mutex;
using std::string;
using std::cerr;
using std::cout;

playbin::playbin()
{
	_playbin = gst_element_factory_make("playbin", "playbin");
	assert(_playbin && "unable to create a playbin element");
}

playbin::~playbin()
{
	gst_element_set_state(_playbin, GST_STATE_NULL);
	gst_object_unref(_playbin);
	_playbin = nullptr;
}

bool playbin::play(std::string const & uri)
{
	g_object_set(_playbin, "uri", uri.c_str(), nullptr);

	GstStateChangeReturn ret = gst_element_set_state(_playbin, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		cerr << "Unable to set the pipeline to the playing state.\n";
		return false;
	}

	return true;
}

bool playbin::stop()
{
	GstStateChangeReturn ret = gst_element_set_state(_playbin, GST_STATE_READY);
	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		cerr << "Unable to set the pipeline to the ready state.\n";
		return false;
	}
	return true;
}

long playbin::position() const
{
	gint64 pos = -1;
	if (!gst_element_query_position(_playbin, GST_FORMAT_TIME, &pos))
		cerr << "warning: could not query current position" << std::endl;
	return (long)pos;
}

long playbin::duration() const
{
	gint64 dur = -1;
	if (!gst_element_query_duration(_playbin, GST_FORMAT_TIME, &dur))
		cerr << "warning: could not query duration" << std::endl;
	return (long)dur;
}

GstElement * playbin::to_gst()
{
	return _playbin;
}

gst_audio_player::gst_audio_player()
	:  _position{0}
	, _duration{0}
	, _playing{false}
	, _position_changed_alert{1}
{}

gst_audio_player::~gst_audio_player()
{}

void gst_audio_player::play(std::string const & uri)
{
	_media = uri;
	_playbin.play(uri);
	loop();
}

string const & gst_audio_player::media() const
{
	return _media;
}


long gst_audio_player::position() const
{
	lock_guard<mutex> lock{_pline_locker};
	return _position;
}

long gst_audio_player::duration() const
{
	lock_guard<mutex> lock{_pline_locker};
	return _duration;
}

bool gst_audio_player::playing() const
{
	lock_guard<mutex> lock{_pline_locker};
	return _playing;
}

void gst_audio_player::stop()
{
	_playbin.stop();
}

void gst_audio_player::on_play(string media, long duration)
{}

void gst_audio_player::on_eos(string media)
{}

void gst_audio_player::on_position_changed(string media, long position)
{}


bool gst_audio_player::handle_message(GstMessage * msg)
{
	switch (GST_MESSAGE_TYPE(msg))
	{
		case GST_MESSAGE_DURATION: {
			cout << "GST_MESSAGE_DURATION" << std::endl;
			return true;
		}

		case GST_MESSAGE_STATE_CHANGED: {
			GstState old_state, new_state, pending_state;
			gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
			if (GST_MESSAGE_SRC(msg) == GST_OBJECT(_playbin.to_gst()))
			{
				cerr << "Pipeline state changed from " << gst_element_state_get_name(old_state)
					<< " to " << gst_element_state_get_name(new_state) << std::endl;

				if (new_state == GST_STATE_PLAYING)
				{
					{
						lock_guard<mutex> lock{_pline_locker};
						_playing = true;
						_position = _playbin.position();
						_duration = _playbin.duration();
						assert(_duration > 0);
					}

					on_play(_media, _duration);
				}
				else
				{
					lock_guard<mutex> lock{_pline_locker};
					_playing = false;
				}
			}
			return true;
		}

		case GST_MESSAGE_EOS:
			on_eos(_media);
			return false;

		case GST_MESSAGE_ERROR: {
			GError * err;
			gchar * debug_info;
			gst_message_parse_error(msg, &err, &debug_info);
			cerr << "Error received from element " << GST_OBJECT_NAME(msg->src)
				<< ": " << err->message << std::endl;
			cerr << "Debugging information: " << (debug_info ? debug_info : "none") << std::endl;
			g_clear_error(&err);
			g_free(debug_info);
			return false;
		}

		default:
			return true;
	}
}

void gst_audio_player::loop()
{
	long prev_pos = 0;

	GstBus * bus = gst_element_get_bus(_playbin.to_gst());
	GstMessage * msg;
	do
	{
		msg = gst_bus_timed_pop_filtered(bus, 100 * GST_MSECOND,
			(GstMessageType)(GST_MESSAGE_STATE_CHANGED|GST_MESSAGE_ERROR|GST_MESSAGE_EOS|GST_MESSAGE_DURATION));

		if (!msg)  // no message means timeout expired
		{
			if (_playing)
			{
				{
					lock_guard<mutex> lock{_pline_locker};
					_position = _playbin.position();
				}

				if (_position_changed_alert.update(_position))
				{
					on_position_changed(_media, _position);
					prev_pos = _position;
				}
			}
		}
	}
	while (!msg || handle_message(msg));

	// clean-up
	if (msg)
		gst_message_unref(msg);

	gst_object_unref(bus);
}

