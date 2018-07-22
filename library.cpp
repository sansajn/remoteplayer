#include <boost/filesystem/operations.hpp>
#include "library.hpp"

using std::vector;

vector<fs::path> library::list_media()
{
	vector<fs::path> result;
	for (fs::directory_entry const & e : fs::recursive_directory_iterator{_libpath})
	{
		if (!fs::is_directory(e))
			result.push_back(e.path());
	}
	return result;
}
