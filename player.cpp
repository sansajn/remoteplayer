#include <algorithm>
#include <iostream>
#include <string>
#include <gst/gst.h>
#include "log.hpp"
#include "player.hpp"

using std::find;
using std::remove;
using std::string;
using std::remove;
using std::cerr;

void observable_audio_player_adapter::register_listener(player_listener * l)
{
	if (find(_listeners.begin(), _listeners.end(), l) == _listeners.end())
		_listeners.push_back(l);
}

void observable_audio_player_adapter::forget_listener(player_listener * l)
{
	remove(_listeners.begin(), _listeners.end(), l);
}


void observable_audio_player_adapter::on_play(string media, long duration)
{
	for (auto * l : _listeners)
		l->on_play(fs::path{media}, duration);
}

void observable_audio_player_adapter::on_position_changed(std::string media, long position)
{
	for (auto * l : _listeners)
		l->on_position_changed(fs::path{media}, position);
}


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

//	for (player_listener * l : _listeners)
//		l->on_queue_changed(player_listener::queue_operation::push, media);
}

void player::register_listener(player_listener * l)
{
	_ap.register_listener(l);
	if (find(_listeners.begin(), _listeners.end(), l) == _listeners.end())
		_listeners.push_back(l);
}

void player::forget_listener(player_listener * l)
{
	_ap.forget_listener(l);
	remove(_listeners.begin(), _listeners.end(), l);
}

void player::loop()
{
	while (true)
	{
		fs::path item;
		_items.wait_and_pop(item);
		if (item.empty())
			break;
		_ap.play("file://" + item.string());
	}

	LOG(trace) << "player shutdown";
}

void player_init(int * argc, char ** argv[])
{
	gst_init(argc, argv);
}
