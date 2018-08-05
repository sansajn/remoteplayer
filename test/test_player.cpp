#include <iostream>
#include "../player.hpp"
#include "../helpers.hpp"

using std::string;
using std::cout;

class foo : public player_listener
{
public:
	foo(player * p) : _p{p}
	{
		p->register_listener(this);
	}

	void on_play(fs::path item, int duration) override
	{
		cout << "on_play(item=" << item << ", duration=" << duration << std::endl;
	}

	void on_position_changed(fs::path item, long position) override
	{
		cout << "on_position_changed(position=" << position << "ns)" << std::endl;
	}

private:
	player * _p;
};

int main(int argc, char * argv[])
{
	player_init(&argc, &argv);

	string input = (argc > 1) ? argv[1] : "../data/test.opus";
	player p;
	p.init();

	foo f{&p};
	p.queue("file://" + pwd() + "/" + input);
	p.join();
	return 0;
}
