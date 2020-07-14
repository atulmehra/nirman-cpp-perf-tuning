#pragma once

#ifndef THREAD_SAFE_MAP_MAP_H
#define THREAD_SAFE_MAP_MAP_H

#include <unordered_map>
#include <queue>
#include <optional>
#include <condition_variable>
#include <mutex>
#include <algorithm>

template <typename KEY, typename T>
class ThreadSafeMap
{
public:
	typedef T ELEMENT_TYPE;
	typedef std::unordered_map<KEY, std::queue<ELEMENT_TYPE > > KEY_ELEM_MAP;
	ThreadSafeMap(const unsigned int init_queue_capacity = 100);
	ThreadSafeMap(ThreadSafeMap&&) = delete;
	ThreadSafeMap(const ThreadSafeMap&) = delete;
	ThreadSafeMap(const ThreadSafeMap&& other) = delete;
	ThreadSafeMap& operator=(const ThreadSafeMap& other) = delete;
	ThreadSafeMap& operator=(const ThreadSafeMap&& other) = delete;

	void push(KEY &key, ELEMENT_TYPE &elem);
	std::optional<ELEMENT_TYPE> try_pop(const KEY);
	//Tood:Optimization possible with swap map algo. Not doing it right now.
	KEY_ELEM_MAP& get_all_messages()
	{
		std::unique_lock<std::mutex>  map_lock(map_mutex);
		is_not_empty_cond.wait(map_lock, [&]() {return internal_map.size() > 0; });

		return internal_map;
	}

private:
	KEY_ELEM_MAP internal_map;
	std::condition_variable is_not_empty_cond;
	std::mutex map_mutex;
	const unsigned int initial_queue_size;

};

template <typename KEY, typename ELEMENT_TYPE>
ThreadSafeMap<KEY, ELEMENT_TYPE>::ThreadSafeMap(const unsigned int init_queue_capacity)
	: initial_queue_size{ init_queue_capacity }
{
	internal_map.reserve(1000);
}

template <typename KEY, typename ELEMENT_TYPE>
std::optional<ELEMENT_TYPE> ThreadSafeMap<KEY, ELEMENT_TYPE>::try_pop(const KEY key)
{
	std::unique_lock<std::mutex>  map_lock(map_mutex);
	is_not_empty_cond.wait(map_lock, [&]() {return internal_map.size() > 0; });
	
	auto loc = internal_map.find(key);
	if (loc != internal_map.end() && loc->second.size())
	{
		auto element = loc->second.front();
		loc->second.pop();
		if (loc->second.size() == 1)
		{
			internal_map.erase(loc);
		}
		return element;
	}
	else
		return std::nullopt;
}



template <typename KEY, typename ELEMENT_TYPE>
void ThreadSafeMap<KEY, ELEMENT_TYPE>::push(KEY &key, ELEMENT_TYPE &elem)
{
	//todo: Add static assert not to take the lvalue
	std::lock_guard<std::mutex>  map_lock(map_mutex);

	internal_map[key].push(elem);
	is_not_empty_cond.notify_all();

	return;
}

#endif
