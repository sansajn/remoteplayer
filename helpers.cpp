#include <fstream>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include <unistd.h>
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

std::string pwd()
{
	// TODO: long linenames support
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	return cwd;
}

watch_alert::watch_alert(int trigger)
	: _trigger{trigger * 1000000000L}
	, _t0{0}
{}

bool watch_alert::update(long position)
{
	assert(position > 0);

	if (_t0 == 0)
	{
		_t0 = position;
		return true;
	}

	if ((position - _t0) >= (long)_trigger)
	{
		_t0 = position + ((position - _t0) - _trigger);
		return true;
	}

	return false;
}

void watch_alert::reset()
{
	_t0 = 0;
}
