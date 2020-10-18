#pragma once
#include <string>
#include <filesystem>
#include <thread>
#include "youtube_dl.hpp"
#include "concurrent_queue.hpp"
#include "event.hpp"

//! \note current implementation do not allow two or more downloader instances
class downloader
{
public:
	downloader(int argc, char * argv[], std::filesystem::path const & download_home,
		std::string const & outtmpl);  // TODO: get rid of argc and argv

	~downloader();

	void download(std::string const & url);

	// signals
	event<std::string, size_t, size_t> progress_update_signal;  //!< (media-id, downloaded-bytes, total-bytes)

private:
	concurrent_queue<std::string> _items;
	youtube_dl _ydl;  // not thread safe
	std::filesystem::path _download_home;
	std::thread _download_worker;
};
