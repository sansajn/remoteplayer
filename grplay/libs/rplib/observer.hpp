#pragma once
#include <algorithm>
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

template <typename Listener>
void observable_with<Listener>::register_listener(Listener * l)
{
	if (find(_listeners.begin(), _listeners.end(), l) == _listeners.end())
		_listeners.push_back(l);
}

template <typename Listener>
void observable_with<Listener>::forget_listener(Listener * l)
{
	std::remove(_listeners.begin(), _listeners.end(), l);
}

template <typename Listener>
std::vector<Listener *> & observable_with<Listener>::listeners()
{
	return _listeners;
}

template <typename Listener>
std::vector<Listener *> const & observable_with<Listener>::listeners() const
{
	return _listeners;
}
