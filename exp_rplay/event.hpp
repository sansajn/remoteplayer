// event system implementation
#pragma once
#include <algorithm>
#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <list>

template <typename ... Args>
class event_handler
{
public:
	using function_type = std::function<void (Args ...)>;
	using id_type = size_t;

	explicit event_handler(function_type const & h);
	event_handler(event_handler && rhs);
	event_handler(event_handler const & rhs);
	void operator()(Args ... args) const;
	id_type id() const;
	bool operator==(event_handler const & rhs) const;
	explicit operator bool() const;
	event_handler & operator=(event_handler && rhs);
	event_handler & operator=(event_handler const & rhs);

private:
	function_type _handler;
	id_type _id;

	static std::atomic_size_t __handler_id_counter;
};

template<typename ... Args>
std::atomic_size_t event_handler<Args ...>::__handler_id_counter{1};  // 0 reserver for invalid id


template <typename ... Args>
class event
{
public:
	using handler_type = event_handler<Args ...>;

	event();
	event(event && rhs);
	event(event const & rhs);
	void call(Args ... args) const;
	std::future<void> call_async(Args ... args) const;
	void operator()(Args ... args) const {call(args ...);}
	typename handler_type::id_type add(handler_type const & h);
	typename handler_type::id_type add(typename handler_type::function_type const & h);
	bool remove(handler_type const & h);
	bool remove_id(typename handler_type::id_type const & id);
	event & operator=(event && rhs);
	event & operator=(event const & rhs);

private:
	using lock_guard_type = std::lock_guard<std::mutex>;
	using handler_collection_type = std::list<handler_type>;

	void call_impl(handler_collection_type const & handlers, Args ... args) const;
	handler_collection_type handlers_copy() const;

	handler_collection_type _handlers;
	mutable std::mutex _handlers_locker;
};


template <typename ... Args>
event<Args ...>::event() {}

template <typename ... Args>
void event<Args ...>::call(Args ... args) const
{
	handler_collection_type handlers = handlers_copy();
	call_impl(handlers, args ...);
}

template <typename ... Args>
std::future<void> event<Args ...>::call_async(Args ... args) const
{
	// TODO: for simple handlers this must be really ineficiont, but still
	// good to have this option there.

	return std::async(std::launch::async,
		[this](Args ... async_args) {
			call(async_args ...);
		},
	args ...);
}

template <typename ... Args>
typename event<Args ...>::handler_type::id_type event<Args ...>::add(handler_type const & h)
{
	lock_guard_type lock{_handlers_locker};
	_handlers.push_back(h);
	return h.id();  // TODO: why return id, caller already have this information ?
}

template <typename ... Args>
typename event<Args ...>::handler_type::id_type event<Args ...>::add(typename handler_type::function_type const & h)
{
	return add(handler_type{h});
}

template <typename ... Args>
bool event<Args ...>::remove(handler_type const & h)
{
	lock_guard_type lock{_handlers_locker};
	auto it = std::find(_handlers.begin(), _handlers.end(), h);
	if (it != _handlers.end())
	{
		_handlers.erase(it);
		return true;
	}

	return false;
}

template <typename ... Args>
bool event<Args ...>::remove_id(typename handler_type::id_type const & id)
{
	lock_guard_type lock{_handlers_locker};

	auto it = std::find_if(_handlers.begin(), _handlers.end(),
		[id](handler_type const & h) {return h.id() == id;}
	);

	if (it != _handlers.end())
	{
		_handlers.erase(it);
		return true;
	}

	return false;
}

template <typename ... Args>
event<Args ...>::event(event && rhs)
{
	lock_guard_type lock{rhs._handlers_locker};
	_handlers = std::move(rhs._handlers);
}

template <typename ... Args>
event<Args ...>::event(event const & rhs)
{
	lock_guard_type lock{rhs._handlers_locker};
	_handlers_locker = rhs._handlers_locker;
}

template <typename ... Args>
event<Args ...> & event<Args ...>::operator=(event && rhs)
{
	lock_guard_type this_lock{_handlers_locker};
	lock_guard_type rhs_lock{rhs._handlers_locker};
	std::swap(_handlers, rhs._handlers);
	return *this;
}

template <typename ... Args>
event<Args ...> & event<Args ...>::operator=(event const & rhs)
{
	lock_guard_type this_lock{_handlers_locker};
	lock_guard_type rhs_lock{rhs._handlers_locker};
	_handlers = rhs._handlers;
	return *this;
}

template <typename ... Args>
void event<Args ...>::call_impl(handler_collection_type const & handlers, Args ... args) const
{
	for (auto const & h : handlers)
		h(args ...);
}

template <typename ... Args>
typename event<Args ...>::handler_collection_type event<Args ...>::handlers_copy() const
{
	std::lock_guard<std::mutex> lock{_handlers_locker};
	return _handlers;
}

template <typename ... Args>
event_handler<Args ...>::event_handler(function_type const & h)
	: _handler{h}
	, _id{__handler_id_counter++}
{}

template <typename ... Args>
void event_handler<Args ...>::operator()(Args ... args) const
{
	if (_handler)
		_handler(args ...);
}

template <typename ... Args>
typename event_handler<Args ...>::id_type event_handler<Args ...>::id() const
{
	return _id;
}

template <typename ... Args>
bool event_handler<Args ...>::operator==(event_handler const & rhs) const
{
	return _id == rhs._id;
}

template <typename ... Args>
event_handler<Args ...>::operator bool() const
{
	return _handler;
}

template <typename ... Args>
event_handler<Args ...>::event_handler(event_handler && rhs)
	: _handler{std::move(rhs._handler)}
	, _id{rhs._id}
{}

template <typename ... Args>
event_handler<Args ...>::event_handler(event_handler const & rhs)
	: _handler{rhs._handler}
	, _id{rhs._id}
{}

template <typename ... Args>
event_handler<Args ...> & event_handler<Args ...>::operator=(event_handler && rhs)
{
	std::swap(_handler, rhs._handler);
	_id = rhs._id;
	return *this;
}

template <typename ... Args>
event_handler<Args ...> & event_handler<Args ...>::operator=(event_handler const & rhs)
{
	_handler = rhs._handler;
	_id = rhs._id;
	return *this;
}
