#include <cassert>
#include <gst/gst.h>
#include "gst_audio_player.hpp"

using std::function;
using std::atomic_bool;
using std::string;


namespace Detail {

template <typename ... Args>
void invoke(function<void (Args ...)> const & f, Args ... args)
{
	if (f)
		f(args ...);
}

//! stateless cancellable play implementation with progress and done (EOS) callback
void play(string const & uri, function<void (int64_t, int64_t)> const & progress_cb,
	function<void ()> const & done_cb, atomic_bool & cancel);

}  // Detail

using play_func_t = void (*)(string const &, function<void (int64_t, int64_t)> const &,
	function<void ()> const &, atomic_bool &);


gst_audio_player::gst_audio_player()
{
	int argc = 0;
	gst_init(&argc, nullptr);
}

void gst_audio_player::play(string const & media, done_cb_type const & done_cb,
	progress_cb_type const & progress_cb)
{
	// deal with previous call
	stop();
	if (_th.joinable())
		_th.join();

	_cancel = false;

	_th = std::thread{static_cast<play_func_t>(Detail::play), media, progress_cb,
		done_cb, std::ref(_cancel)};
}

void gst_audio_player::stop()
{
	_cancel = true;
}

void gst_audio_player::join()
{
	if (_th.joinable())
		_th.join();
}


namespace Detail {

void play(string const & uri, function<void (int64_t, int64_t)> const & progress_cb,
	function<void ()> const & done_cb, atomic_bool & cancel)
{
	GstElement * playbin = gst_element_factory_make("playbin", "playbin");
	assert(playbin && "unable to create a playbin element");

	// insert media
	g_object_set(playbin, "uri", uri.c_str(), nullptr);

	// play
	GstStateChangeReturn ret = gst_element_set_state(playbin, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE)
		return;  // unable to play
	assert(ret == GST_STATE_CHANGE_ASYNC);

	// wait till ends
	GstBus * bus = gst_element_get_bus(playbin);

	GstMessage * msg;
	do {
		msg = gst_bus_timed_pop_filtered(bus, 100 * GST_MSECOND,
			(GstMessageType)(GST_MESSAGE_ERROR|GST_MESSAGE_EOS));

		if (msg)
		{
			if (msg->type == GST_MESSAGE_EOS)
				invoke(done_cb);
		}
		else
		{
			GstState state, pending;
			gst_element_get_state(playbin, &state, &pending, GST_CLOCK_TIME_NONE);
			if (state != GST_STATE_PLAYING)
				continue;

			int64_t pos = -1;
			if (!gst_element_query_position(playbin, GST_FORMAT_TIME, &pos))
				assert(0 && "unable to query playbin position");

			int64_t dur = -1;
			if (!gst_element_query_duration(playbin, GST_FORMAT_TIME, &dur))
				assert(0 && "unable to query playbin duration");

			invoke(progress_cb, pos, dur);
		}
	}
	while (!msg && !cancel);

	if (msg)
		gst_message_unref(msg);

	gst_object_unref(bus);

	gst_element_set_state(playbin, GST_STATE_NULL);
	gst_object_unref(playbin);
}

}  // Detail
