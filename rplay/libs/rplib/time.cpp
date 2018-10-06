#include "time.hpp"

namespace rpl {

using std::string;

string timestamp()
{
	return to_iso_string(now());
}

ptime to_ptime(string const & iso_t)
{
	return from_iso_string(iso_t);
}

ptime now()
{
	return microsec_clock::universal_time();
}

}  // rpl
