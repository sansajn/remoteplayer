#pragma once
#include <vector>
#include <string>

class library
{
public:
	library(std::string const & library_home);
	std::vector<std::string> list_media() const;

private:
	std::string const _home;
};
