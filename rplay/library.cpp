#include <boost/filesystem/operations.hpp>
#include "fs.hpp"
#include "library.hpp"

using std::string;
using std::vector;

library::library(std::string const & library_home)
	: _home{library_home}
{}

vector<string> library::list_media() const
{
	vector<string> result;
	for (fs::directory_entry const & e : fs::recursive_directory_iterator{_home})
	{
		if (!fs::is_directory(e))
			result.push_back(e.path().string());
	}
	return result;
}
