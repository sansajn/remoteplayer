#pragma once
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

inline std::string cwd()
{
	char buf[1024];
	getcwd(buf, sizeof(buf));
	return buf;
}
