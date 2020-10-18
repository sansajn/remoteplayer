#pragma once
#include <string>
#include <filesystem>

class config
{
public:
	config();
	config(int argc, char * argv[]);

	// configurable
	unsigned short port;  //!< zmq interface port
	std::string media_home,
		log_file,
		outtmpl;  //!< downloaded media name template e.g. "%(title)s-%(id)s.%(ext)s"
	std::filesystem::path download_directory;

	// not configurable
	std::filesystem::path program_name;  //!< argv[0]
};
