#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

class playlist
{
public:
	playlist();
	std::string wait_next(); /*const*/
	void add(std::string const & item);
	void remove(std::string const & item);
	void clear();
	size_t size() const;
	std::vector<std::string> items() const;
	void repeat();

private:
	std::vector<std::string> _items;
	size_t _item_idx;  //!< current item index
	mutable std::mutex _items_locker;
	std::condition_variable _new_item_cond;
};
