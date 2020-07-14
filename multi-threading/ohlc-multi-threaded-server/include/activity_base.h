#ifndef ACTIVITY_BASE_H
#define ACTIVITY_BASE_H

#include <chrono>
#include <iostream>
#include <queue>

template <typename KEY, typename ELEMENT_TYPE>
class ThreadSafeMap;

template <typename KEY, typename ELEMENT_TYPE>
class ActivityBase
{
public:
	//typedef ThreadSafeMap<std::string, Trade > MAP_TYPE;
	typedef ThreadSafeMap<KEY, ELEMENT_TYPE > MAP_TYPE;
	ActivityBase(MAP_TYPE &data_map) : shared_data{ data_map }, stop_flag{ false }
	{}

	//Non copyable, non assignable
	ActivityBase(const ActivityBase &) = delete;
	ActivityBase(ActivityBase &&) = delete;
	ActivityBase& operator = (const ActivityBase &) = delete;
	ActivityBase& operator = (ActivityBase &&) = delete;

	inline void stop()
	{
		stop_flag = true;
	}
	virtual void run(/*CALLABLE f*/) = 0;
	
protected:
	//typedef ThreadSafeMap<std::string, Trade > MAP_TYPE;
	MAP_TYPE &shared_data;
	volatile bool stop_flag;
};

#endif

