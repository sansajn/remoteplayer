#include <fstream>
#include <sstream>
#include "helpers.hpp"

using std::ofstream;
using std::ifstream;
using std::ostringstream;

bool save_to_file(std::string const & fname, std::string const & content)
{
	ofstream fout{fname};
	if (!fout.is_open())
		return false;

	fout.write(content.c_str(), content.size());  // save content

	return true;
}

bool load_from_file(std::string const & fname, std::string & content)
{
	ifstream fin{fname};
	if (!fin.is_open())
		return false;

	ostringstream ss;
	ss << fin.rdbuf();
	content = ss.str();

	return true;
}
