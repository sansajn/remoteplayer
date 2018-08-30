#include <fstream>
#include <sstream>
#include "io.hpp"

using std::string;
using std::ofstream;
using std::ifstream;
using std::ostringstream;


string read_file(string const & fname)
{
	ifstream in{fname};
	if (!in.is_open())
		throw std::runtime_error{string{"unable to open '"} + fname + "' file"};

	ostringstream ss;
	ss << in.rdbuf();

	return ss.str();
}

void save_file(string const & fname, string const & data)
{
	ofstream out(fname);
	if (!out.is_open())
		throw std::runtime_error{string{"unable to create '"} + fname + "' file"};

	out << data;

	out.close();
}
