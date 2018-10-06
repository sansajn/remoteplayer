#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

class playlist  //!< thread safe playlist implementation
{
public:
	playlist();
	std::string wait_next(); /*const*/
	bool try_next(std::string & item);
	void set_current_item(size_t idx);
	void add(std::string const & item);
	void remove(size_t idx);
	void clear();
	size_t size() const;
	std::vector<std::string> items() const;
	size_t current_item_idx() const;
	std::string item(size_t idx) const;
	void repeat();

private:
	std::vector<std::string> _items;
	size_t _item_idx;  //!< current item index
	mutable std::mutex _items_locker;
	std::condition_variable _new_item_cond;
};
