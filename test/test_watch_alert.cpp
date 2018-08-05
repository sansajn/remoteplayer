#include <limits>
#include <iostream>
#include "../helpers.hpp"

using std::cout;

int main(int argc, char * argv[])
{
	watch_alert every_1s{1};
	for (long t = 1; t < std::numeric_limits<long>::max(); ++t)
	{
		if (every_1s.update(t))
			cout << "1s passed in t=" << t << "ns" << std::endl;
	}
	cout << "done!\n";
	return 0;
}
