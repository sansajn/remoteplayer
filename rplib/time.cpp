#include "time.hpp"

namespace rpl {

using std::string;

string timestamp()
{
	return to_iso_string(microsec_clock::universal_time());
}

ptime to_ptime(string const & iso_t)
{
	return from_iso_string(iso_t);
}

}  // rpl
