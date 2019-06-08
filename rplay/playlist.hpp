#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

class playlist  //!< thread safe playlist implementation
{
public:
	static size_t const npos = static_cast<size_t>(-1);

	playlist();
	std::string wait_next(); /*const*/
	bool try_next(std::string & item);
	void set_current_item(size_t idx);
	void add(std::string const & item);
	void remove(size_t idx);
	void remove(std::vector<size_t> const & indices);
	void move(size_t from_idx, size_t to_idx);
	void clear();
	size_t size() const;
	std::vector<std::string> items() const;
	size_t current_item_idx() const;  //!< returns one item ahead
	std::string item(size_t idx) const;
	void repeat();
	void shuffle(bool state);
	bool shuffle() const;

private:
	size_t next_item_idx();

	std::vector<std::string> _items;
	size_t _item_idx;  //!< current item index or npos
	bool _shuffle;
	mutable std::mutex _items_locker;
	std::condition_variable _new_item_cond;
};
