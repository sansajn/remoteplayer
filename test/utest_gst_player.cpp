// gst audio player unit test module
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

class foo_player : public gst_audio_player
{
public:
	int counter = 0;

private:
	void on_eos(string media)
	{
		++counter;
	}
};

TEST_CASE("we cam play media", "[gst_audio_player]")
{
	gst_init_once();
	foo_player p;
	p.play("file://" + pwd() + MEDIA_1s);
	REQUIRE(p.counter == 1);
}

TEST_CASE("we cam play multiple media", "[gst_audio_player]")
{
	gst_init_once();
	foo_player p;
	p.play("file://" + pwd() + MEDIA_1s);
	REQUIRE(p.counter == 1);
	p.play("file://" + pwd() + MEDIA_1s);
	REQUIRE(p.counter == 2);
}
