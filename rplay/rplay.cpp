// remote player implementation
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <memory>
#include <iostream>
#include <csignal>
#include "inotify-cpp/NotifierBuilder.h"
#include "rplib/log.hpp"
#include "player.hpp"
#include "library.hpp"
#include "downloader.hpp"
#include "zmq_interface.hpp"
#include "config.hpp"
#include "version.hpp"

using std::string;
using std::vector;
using std::unique_ptr;
using std::cout, std::clog, std::endl;
using namespace std::chrono_literals;
namespace this_thread = std::this_thread;

inotify::NotifierBuilder * __inotify = nullptr;
bool __quit = false;

static void custom_signal_handler(int signal_id)
{
	signal(signal_id, SIG_DFL);
	clog << "signal handler" << endl;
	assert(__inotify);
	__inotify->stop();
	__quit = true;
}

int main(int argc, char * argv[])
{
	config conf{argc, argv};

	if (!conf.log_file.empty())
	{
		rpl::log_to_file(conf.log_file);

		LOG(info) << software_name() << " " << software_version() << " (" << software_build() << ")";
		LOG(info) << "listenning on tcp://*:" << conf.port;
		LOG(info) << "media-home: " << conf.media_home;
		LOG(info) << "log-file: " << conf.log_file;
	}
	else
		rpl::log_to_console();

	cout << software_name() << " " << software_version() << " (" << software_build() << ")\n";
	cout << "listenning on tcp://*:" << conf.port << "\n";
	cout << "media-home: " << conf.media_home << "\n";
	if (!conf.log_file.empty())
		cout << "log-file: " << conf.log_file << "\n";

	library lib{conf.media_home};
	player play;
	downloader down{argc, argv, conf.media_home / conf.download_directory, conf.outtmpl};
	zmq_interface iface{conf.port, &lib, &play, &down};

	iface.run();
	play.start();

	// TODO: move inotify to its own thread
	auto notifycation_handler = [&iface](inotify::Notification notification) {
		iface.send_library_update();
		if (notification.event == inotify::Event::close_write)
			LOG(info) << "media " << notification.path << " added";
		else if (notification.event == inotify::Event::remove)
			LOG(info) << "media " << notification.path << " removed";
	};

	vector<inotify::Event> events = {
		inotify::Event::close_write, inotify::Event::remove};

	bool media_notify = false;
	inotify::NotifierBuilder notifier;
	if (fs::exists(conf.media_home))
	{
		notifier = inotify::BuildNotifier()
			.watchPathRecursively(conf.media_home)
			.onEvents(events, notifycation_handler);
		media_notify = true;

		__inotify = &notifier;
	}

	signal(SIGTERM, &custom_signal_handler);
	signal(SIGINT, &custom_signal_handler);

	while (!__quit)
	{
		if (media_notify)
			notifier.runOnce();  // BUG: this is blocking, why? <- it is waiting for one event
		this_thread::sleep_for(10ms);
	}

	iface.stop();

	clog << "done!" << endl;

	return 0;
}  // BUG: proper shutdown is not working (see ~player)
