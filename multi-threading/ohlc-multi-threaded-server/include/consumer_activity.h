#ifndef CONSUMER_ACTIVITY_H
#define CONSUMER_ACTIVITY_H

#include <chrono>
#include <iostream>
#include <thread>
#include <array>
#include "activity_base.h"


template <class CALLABLE >
class ConsumerActivity : public ActivityBase <std::string, Trade >
{
public:
	ConsumerActivity(MAP_TYPE &data_map, CALLABLE function) : ActivityBase{ data_map },
		callback{ function }, current_subscription_count{0}
	{
	}

	void add_subscription(std::string symbol)
	{
		if (current_subscription_count == MAX_SUBSCRIPTIONS)
			throw std::exception("Max count reached");
		
		subscriptions[current_subscription_count++] = symbol;
	}
	
	virtual void run() override
	{
		std::cout << "LOG|INFO: Starting Consumer Activity" << std::endl;
		while (!stop_flag)
		{
			if (stop_flag)
				break;
			for (const auto &current_subscription_sym : subscriptions)
			{
				if (stop_flag)
					break;
				while (auto trade = shared_data.try_pop(current_subscription_sym))
				{
					callback(*trade);
				}
			}
		}
	}
private:
	CALLABLE callback;
	static int constexpr MAX_SUBSCRIPTIONS = 100;
	std::array<std::string, MAX_SUBSCRIPTIONS> subscriptions;
	uint16_t current_subscription_count;
	
};

#endif
