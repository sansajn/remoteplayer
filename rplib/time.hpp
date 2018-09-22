#pragma once
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace rpl {

using namespace boost::posix_time;

std::string timestamp();
ptime to_ptime(std::string const & iso_t);

}  // rpl
