// remote player implementation
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <memory>
#include <iostream>
#include "inotify-cpp/NotifierBuilder.h"
#include "rplib/log.hpp"
#include "player.hpp"
#include "zmq_interface.hpp"
#include "library.hpp"
#include "config.hpp"
#include "version.hpp"

using std::string;
using std::vector;
using std::unique_ptr;
using std::cout;
using std::chrono::milliseconds;
namespace this_thread = std::this_thread;

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
	zmq_interface iface{conf.port, &lib, &play};

	iface.run();
	play.start();

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
	}

	while (true)
	{
		if (media_notify)
			notifier.runOnce();

		this_thread::sleep_for(milliseconds{10});
	}

	iface.join();

	return 0;
}
