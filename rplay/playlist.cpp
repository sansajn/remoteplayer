#include <algorithm>
#include <stdexcept>
#include <cassert>
#include "rplib/random.hpp"
#include "playlist.hpp"

using std::string;
using std::to_string;
using std::vector;
using std::find;
using std::mutex;
using std::lock_guard;
using std::unique_lock;


playlist::playlist()
	: _item_idx{npos}
	, _shuffle{false}
{}

bool playlist::try_next()
{
	lock_guard<mutex> lock{_items_locker};
	size_t idx = next_item_idx();
	if (idx >= npos)
		return false;

	return true;
}

void playlist::set_current_item(size_t idx)
{
	if (idx >= _items.size())
		throw std::out_of_range{"playlist index (" + to_string(idx) + ") out of range"};

	lock_guard<mutex> lock{_items_locker};
	_item_idx = idx;
	_new_item_cond.notify_one();
}


void playlist::add(string const & item)
{
	lock_guard<mutex> lock{_items_locker};
	_items.push_back(item);
	_new_item_cond.notify_one();
}

void playlist::remove(size_t idx)
{
	lock_guard<mutex> lock{_items_locker};
	if (idx >= _items.size())
		throw std::out_of_range{"playlist index (" + to_string(idx) + ") out of range"};

	_items.erase(_items.begin() + (int)idx);

	if (idx < _item_idx)
		_item_idx -= 1;

	if (_items.empty())
		_item_idx = npos;
}

void playlist::remove(vector<size_t> const & indices)
{
	vector<size_t> sorted = indices;
	std::sort(sorted.begin(), sorted.end(), std::greater<size_t>{});

	if (sorted.front() < (size_t)0 || sorted.back() > _items.size())
		throw std::out_of_range{"playlist index out of range"};

	lock_guard<mutex> lock{_items_locker};

	for (size_t idx : sorted)
	{
		if (idx < _item_idx)
			_item_idx -= 1;
		_items.erase(_items.begin() + (int)idx);
	}

	if (_items.empty())
		_item_idx = npos;
}

void playlist::move(size_t from_idx, size_t to_idx)
{
	lock_guard<mutex> lock{_items_locker};

	if (from_idx == to_idx ||
		from_idx >= _items.size() || to_idx >= _items.size())
	{
		return;
	}

	string item = _items[from_idx];

	if (from_idx < to_idx)
		for (size_t i = from_idx; i < to_idx; ++i)
			_items[i] = _items[i+1];
	else if (from_idx > to_idx)
		for (size_t i = from_idx; i > to_idx; --i)
			_items[i] = _items[i-1];

	_items[to_idx] = item;
}

void playlist::clear()
{
	lock_guard<mutex> lock{_items_locker};
	_items.clear();
}

size_t playlist::size() const
{
	lock_guard<mutex> lock{_items_locker};
	return _items.size();
}

vector<string> playlist::items() const
{
	lock_guard<mutex> lock{_items_locker};
	return _items;
}

size_t playlist::current_item_idx() const
{
	lock_guard<mutex> lock{_items_locker};
	if (_item_idx <= _items.size())
		return _item_idx;
	else
		return npos;
}

string playlist::item(size_t idx) const
{
	lock_guard<mutex> lock{_items_locker};
	return _items[idx];
}

void playlist::repeat()
{
	lock_guard<mutex> lock{_items_locker};
	if (!_items.empty())
	{
		_item_idx = 0;
		_new_item_cond.notify_one();
	}
	else
		_item_idx = npos;
}

void playlist::shuffle(bool state)
{
	lock_guard<mutex> lock{_items_locker};
	_shuffle = state;
}

bool playlist::shuffle() const
{
	lock_guard<mutex> lock{_items_locker};
	return _shuffle;
}

size_t playlist::next_item_idx()
{
	if (_shuffle)
	{
		if (!_items.empty())
		{
			_item_idx = rand_int() % _items.size();
			return _item_idx;
		}
		else
		{
			assert(0);
			return 0;
		}
	}
	else
	{
		if (!_items.empty())
		{
			if (_item_idx != npos)
			{
				_item_idx += 1;
				if (_item_idx >= _items.size())
					_item_idx = npos;
			}
			else
				_item_idx = 0;

			return _item_idx;
		}
		else
		{
			assert(0);
			return 0;
		}
	}
}
