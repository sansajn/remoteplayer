#pragma once
#include <string>

class config
{
public:
	config();
	config(int argc, char * argv[]);

	std::string media_home;
	unsigned short port;  //!< zmq interface port
	std::string log_file;
};
