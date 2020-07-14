#ifndef OHLC_SERVER_H
#define OHLC_SERVER_H
#include <future>
#include <memory>
#include <time.h>
#include <tuple>
//Boost timer to print performance
//#include <boost/timer/timer.hpp>
#include "../include/thread_safe_map.h"
#include "../include/json_trade_data_source.h"
#include "../include/consumer_activity.h"
#include "../include/bar_data.h"
#include "../include/client_activity.h"

class OHLCServer
{
public:
	void init(std::string subscriptions_file_path);
	OHLCServer(std::string data_source_file_path, bool empty_bars);
	void stop();

	void add_subscription(std::string subscription);

	//Typedefs
	typedef typename std::function<void(Trade&)> CALLABLE_TYPE;
	typedef ConsumerActivity<CALLABLE_TYPE> ACTIVITY_TYPE;

	typedef std::shared_ptr<ACTIVITY_TYPE> SHARED_PTR_ACTIVITY_TYPE;
	typedef std::string SYMBOL_TYPE;
	typedef time_t TIME;

	typedef typename std::function<void(OHLC&)> CLIENT_ACTIVITY_CALLBACK;
	typedef ClientActivity<CLIENT_ACTIVITY_CALLBACK> CLIENT_ACTIVITY_TYPE;


	std::shared_ptr<CLIENT_ACTIVITY_TYPE> & get_client_activity()
	{
		return client_activity;
	}

private:
	
	//Worker1 - Data source thread
	std::shared_ptr<JSONTradeDataSource> data_source;
	std::future<void> data_source_thread;


	//Worker 2 - Consumes and passes message to OHLCServer for FSM
	SHARED_PTR_ACTIVITY_TYPE data_consumer;	
	std::future<void> data_consumer_thread;

	//Worker 3 - Client Thread
	std::shared_ptr<CLIENT_ACTIVITY_TYPE> client_activity;
	std::future<void> data_client_thread;

	/////////// Shared data    ///////////////
	//Shared data between data source thread and FSM thread
	ThreadSafeMap<SYMBOL_TYPE, Trade > recieved_messages_thread_safe;
	//Shared data between FSM thread & client thread
	ThreadSafeMap<SYMBOL_TYPE, OHLC> client_messages_thread_safe;

	//Handle incoming trades and update state
	void handle_new_trade(Trade &);	
	inline void update_ohlc(Trade &trade) noexcept;

	//States
	static constexpr uint16_t BAR_INTERVAL = 15;
	enum symbol_state_elements { STATE, FIRST_UPDATE_AT};
	// Symbol --> tuple<States of current bar count
	std::unordered_map<SYMBOL_TYPE, std::tuple<OHLC, TIME> > symbols_states;

	//Subscriptions. In future can recieve subscr from multiple client threads.
	//Limit total load.
	static int constexpr MAX_SUBSCRIPTIONS = 100;
	//std::unordered_set subscriptions;
	//std::array<std::string, MAX_SUBSCRIPTIONS> subscriptions;
	uint16_t current_subscription_count;

	std::shared_ptr<OHLCServer> ohlc_shared;
	//boost::timer::cpu_timer performance_stats_server;
	void wait();

	bool send_empty_bars;
	volatile bool stop_flag;

};

#endif