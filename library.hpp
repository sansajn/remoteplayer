#pragma once
#include <vector>
#include "fs.hpp"

class library
{
public:
	library(fs::path const & p)
		: _libpath{p}
	{}

	std::vector<fs::path> list_media();

private:
	fs::path _libpath;
};
