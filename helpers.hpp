#pragma once
#include <string>

bool save_to_file(std::string const & fname, std::string const & content);
bool load_from_file(std::string const & fname, std::string & content);
