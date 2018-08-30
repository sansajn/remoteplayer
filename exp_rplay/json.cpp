#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include "io.hpp"
#include "json.hpp"

using std::stringstream;

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
