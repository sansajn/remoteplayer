#pragma once
#include <vector>

// TODO: implement notify function
template <typename Listener>
class observable_with
{
public:
	void register_listener(Listener * l);  // TODO: find something better then register
	void forget_listener(Listener * l);
	std::vector<Listener *> & listeners();
	std::vector<Listener *> const & listeners() const;

private:
	std::vector<Listener *> _listeners;
};
