// gst audio player unit test module
#include <iostream>
#include <catch.hpp>
#include "../gst_audio_player.hpp"

#define MEDIA_1s "/1s.opus"

using std::string;

static void gst_init_once()
{
	static bool first_time = true;

	if (first_time)
	{
		int argc = 0;
		char ** argv = nullptr;
		gst_init(&argc, &argv);
		first_time = false;
	}
}

class counted_player_impl : public gst_audio_player
{
public:
	int counter = 0;

private:
	void on_eos(string media) override
	{
		++counter;
	}
};

class position_change_player_impl : public gst_audio_player
{
public:
	int counter = 0;

private:
	void on_position_changed(std::string media, long position) override
	{
		++counter;
	}
};


TEST_CASE("we cam play media", "[gst_audio_player]")
{
	gst_init_once();
	counted_player_impl p;
	p.play("file://" + pwd() + MEDIA_1s);
	REQUIRE(p.counter == 1);
}

TEST_CASE("we cam play multiple media", "[gst_audio_player]")
{
	gst_init_once();
	counted_player_impl p;
	p.play("file://" + pwd() + MEDIA_1s);
	REQUIRE(p.counter == 1);
	p.play("file://" + pwd() + MEDIA_1s);
	REQUIRE(p.counter == 2);
}

TEST_CASE("position is changeing while playing", "[gst_audio_player]")
{
	gst_init_once();
	position_change_player_impl p;
	p.play("file://" + pwd() + MEDIA_1s);
	REQUIRE(p.counter == 1);
}

TEST_CASE("position is changeing while playing multiple media", "[gst_audio_player]")
{
	gst_init_once();
	position_change_player_impl p;
	p.play("file://" + pwd() + MEDIA_1s);
	REQUIRE(p.counter == 1);
	p.play("file://" + pwd() + MEDIA_1s);
	REQUIRE(p.counter == 2);
}
