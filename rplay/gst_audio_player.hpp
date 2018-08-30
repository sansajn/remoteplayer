#pragma once
#include <functional>
#include <atomic>
#include <thread>
#include <string>

/*! async player based on gstreamer library
\code
player p;
p.play("hello.mp3", [](){cout << "done\n";}, [](int64_t pos, int64_t dur) {
	cout << "progress=" << pos << "/" << dur << std::endl;});
p.join();
\endcode */
class gst_audio_player
{
public:
	using done_cb_type = std::function<void ()>;
	using progress_cb_type = std::function<void (int64_t, int64_t)>;

	gst_audio_player();

	void play(std::string const & media, done_cb_type const & done_cb = done_cb_type{},
		progress_cb_type const & progress_cb = progress_cb_type{});

	void stop();
	void join();

private:
	std::thread _th;
	std::atomic_bool _cancel;
};
