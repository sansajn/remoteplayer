#include <atomic>
#include <filesystem>
#include <system_error>
#include "downloader.hpp"
#include "youtube_dl.hpp"
#include "rplib/log.hpp"

using std::string;
using std::filesystem::path,
	std::filesystem::exists,
	std::filesystem::create_directories,
	std::filesystem::copy_file,
	std::filesystem::remove;
using std::error_code;
using std::atomic_bool;

static void download_worker(youtube_dl & ydl, concurrent_queue<string> & items,
	path media_home);

static bool move_to_library(string const & file_name, path const & downlaod_home,
	error_code & ec);


downloader::downloader(int argc, char * argv[], path const & download_home,
	string const & outtmpl)
		: _ydl{progress_update_signal, argc, argv, "/tmp/" + outtmpl}
		, _download_home{download_home}
{
	LOG(trace) << "downloader initialized";
}

downloader::~downloader()
{
	// clear download queue
	string dummy;
	while (_items.try_pop(dummy))
		continue;

	if (_download_worker.joinable())  // just wait for downloader to finish all downloads
		_download_worker.join();

	LOG(debug) << "downloader down";
}

atomic_bool __worker_done = false;

void downloader::download(string const & url)
{
	_items.push(url);

	if (__worker_done && _download_worker.joinable())
		_download_worker.join();  // make thread !joinable()

	if (!_download_worker.joinable())
	{
		__worker_done = false;
		_download_worker = std::thread{download_worker, std::ref(_ydl),
			std::ref(_items), _download_home};
	}
}

void download_worker(youtube_dl & ydl, concurrent_queue<string> & items,
	path media_home)
{
	string url;
	while (items.try_pop(url))
	{
		LOG(info) << "downloading " << url;

		string file_name;
		if (ydl.download_audio(url, file_name))
		{
			LOG(info) << "download '" << file_name << "' finished";

			// TODO: convert to native container (opus, mp3, ...)

			error_code ec;
			if (!move_to_library(file_name, media_home, ec))
			{
				LOG(error) << "unable to move \"" << file_name << "\" to " << media_home
					<< " directory, what: " << ec.message();
			}
		}
		else
			LOG(error) << "download failed!";
	}

	__worker_done = true;

	LOG(debug) << "download worker done";
}

bool move_to_library(string const & file_name, path const & download_home,
	error_code & ec)
{
	// replace /tmp with $media_home
	path p{file_name};
	auto it = p.begin();
	++it;  // skip root '/'
	++it;  // skip tmp
	assert(it != p.end());

	path dst{download_home};
	for (; it != p.end(); ++it)
		dst /= *it;

	// create paren directory if necessary
	if (!exists(download_home))
		create_directories(download_home, ec);

	// we can't use rename there, otherwise 'Invalid cross-device link' error
	if (!ec)
		copy_file(file_name, dst, ec);

	if (!ec)
		remove(file_name, ec);

	return ec ? false : true;
}
