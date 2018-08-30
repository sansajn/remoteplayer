#include <cstdlib>
#include <unistd.h>
#include "fs.hpp"

using std::string;

string cwd()
{
	char buf[1024];
	getcwd(buf, sizeof(buf));
	return buf;
}

string home_dir()
{
	return getenv("HOME");
}
