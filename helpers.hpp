#pragma once
#include <string>
#include "json.hpp"

std::string read_file(std::string const & fname);
void save_file(std::string const & fname, std::string const & data);

jtree read_json_file(std::string const & fname);
void save_json_file(std::string const & fname, jtree const & root);
