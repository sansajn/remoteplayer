#include <iostream>
#include <string>
#include <gst/gst.h>
#include "player.hpp"

using std::string;
using std::cerr;

player::player()
{}

void player::init()
{
	_t = std::thread{&player::loop, this};
}

void player::quit()
{
	_items.push(fs::path{});
	join();
}

void player::join()
{
	_t.join();
}

void player::queue(fs::path const & media)
{
	_items.push(media);
}

void player::loop()
{
	while (true)
	{
		fs::path item;
		_items.wait_and_pop(item);
		if (item.empty())
			break;
		play(item);
	}
}

void player::play(fs::path const & media)
{
	GstElement * playbin = gst_element_factory_make("playbin", "playbin");
	assert(playbin && "unable to create a playbin element");
	string uri = "file://" + media.string();
	g_object_set(playbin, "uri", uri.c_str(), nullptr);

	// start playing
	GstStateChangeReturn ret = gst_element_set_state(playbin, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		cerr << "Unable to set the pipeline to the playing state.\n";
		gst_object_unref(playbin);
		throw std::runtime_error{"[player]: Unable to set the pipeline to the playing state."};
	}

	// wait until error or EOS
	GstBus * bus = gst_element_get_bus(playbin);
	GstMessage * msg = gst_bus_timed_pop_filtered(bus, (GstClockTime)GST_CLOCK_TIME_NONE,
		(GstMessageType)(GST_MESSAGE_ERROR|GST_MESSAGE_EOS));

	// clean-up
	if (msg)
		gst_message_unref(msg);

	gst_object_unref(bus);
	gst_element_set_state(playbin, GST_STATE_NULL);
	gst_object_unref(playbin);
}

void player_init(int * argc, char ** argv[])
{
	gst_init(argc, argv);
}
