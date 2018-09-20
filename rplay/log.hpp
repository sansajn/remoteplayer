#pragma once
#include <string>
#include <boost/log/trivial.hpp>

#define LOG(lvl) BOOST_LOG_TRIVIAL(lvl)

void log_to_console();
void log_to_file(std::string const & log_file);
