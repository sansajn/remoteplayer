#pragma once
#include <zmqu/json.hpp>

jtree read_json_file(std::string const & fname);
void save_json_file(std::string const & fname, jtree const & root);
