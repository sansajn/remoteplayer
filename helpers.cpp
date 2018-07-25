#include <fstream>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include "helpers.hpp"

using std::string;
using std::ofstream;
using std::ifstream;
using std::ostringstream;
using std::stringstream;

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

jtree read_json_file(std::string const & fname)
{
	jtree result;
	stringstream ss{read_file(fname)};
	boost::property_tree::read_json(ss, result);
	return result;
}

void save_json_file(std::string const & fname, jtree const & root)
{
	boost::property_tree::write_json(fname, root);
}
