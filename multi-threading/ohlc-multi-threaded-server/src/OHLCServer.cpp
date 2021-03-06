// OHLCServer.cpp : Defines the entry point for the console application.
//
#include <string>
#include "../include/ohlc_server.h"


void OHLCServer::init(std::string subscriptions_file_path)
{
	//ohlc_shared = std::shared_ptr<OHLCServer>(this);
	//client_activity->init(ohlc_shared, subscriptions_file_path);
	data_client_thread = std::async(std::launch::async, &CLIENT_ACTIVITY_TYPE::run, client_activity);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	data_consumer_thread = std::async(std::launch::async, &ACTIVITY_TYPE::run, data_consumer);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	data_source_thread = std::async(std::launch::async, &JSONTradeDataSource::run, data_source);
	
}

void OHLCServer::wait()
{
	data_source_thread.wait();
	//data_consumer->stop();
	//client_activity->stop();
	data_consumer_thread.wait();
	data_client_thread.wait();
}

void OHLCServer::stop()
{
	std::cout << "LOG|INFO: Stopping the threads now" << std::endl;
	data_source->stop();
	data_consumer->stop();
	client_activity->stop();
	wait();
}

void OHLCServer::add_subscription(std::string symbol)
{
	if (current_subscription_count == MAX_SUBSCRIPTIONS)
			throw std::exception("Max count reached");
	data_consumer->add_subscription(symbol);
	
	//If subsciption for first time, then init state
	if (symbols_states.find(symbol) == symbols_states.end())
	{
		symbols_states[symbol] = std::make_tuple(OHLC(symbol), 0);
		++current_subscription_count;
	}
}

OHLCServer::OHLCServer(std::string data_source_file_path, bool empty_bars) : 
	data_source{ std::make_shared<JSONTradeDataSource>(recieved_messages_thread_safe, data_source_file_path) },
	data_consumer{ std::make_shared<ACTIVITY_TYPE>(recieved_messages_thread_safe, [&](Trade &t) { handle_new_trade(t); }) },
	client_activity {std::make_shared<CLIENT_ACTIVITY_TYPE>(client_messages_thread_safe,
		[&](OHLC &bar_data) {client_activity->handle_client_message(bar_data); }) },
		send_empty_bars{ empty_bars }, stop_flag{false}
	
{
	std::cout << "************* Log format1 : LOG|LOG_LEVEL|RECIEVED:: SYMBOL|BAR_COUNT|TS2|CURRENT_BAR_CLOSE_TIME|PRICE|QUANTITY *************" << std::endl;
	std::cout << "************* Log format2 : LOG|LOG_LEVEL|BAR_STATE:: SYMBOL|BAR_COUNT|TS2|CURRENT_BAR_CLOSE_TIME|O|L|H|C|QUANTITY *************" << std::endl;
	std::cout << "************* Log format3 : LOG|LOG_LEVEL|CLIENT_MSG: (Self explanatory) *************" << std::endl;
	std::cout << "************* Final client messages sent are written to Output.txt in current directory *************" << std::endl;
}

void OHLCServer::update_ohlc(Trade &trade) noexcept
{
	time_t seconds_time = trade.timestamp / 1000000000;
	auto &symbol_state = symbols_states[trade.symbol];
	auto & first_bar_start_for_symbol_at = std::get<FIRST_UPDATE_AT>(symbol_state);
	auto & state = std::get<STATE>(symbol_state);
	if (first_bar_start_for_symbol_at == 0)
	{
		first_bar_start_for_symbol_at = seconds_time;
		state.open = trade.price;
	}
	uint32_t bar_count = ((seconds_time - first_bar_start_for_symbol_at) / BAR_INTERVAL) + 1;
	time_t bar_end_time = first_bar_start_for_symbol_at + BAR_INTERVAL;

	/// **************************** Logging Purpose *******************************
	struct tm  ts;
	char       current_trade_time[80], bar_close_time[80];


	// Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
	ts = *gmtime(&seconds_time);
	strftime(current_trade_time, sizeof(current_trade_time), "%Y-%m-%d %H:%M:%S", &ts);
	ts = *gmtime(&bar_end_time);
	strftime(bar_close_time, sizeof(bar_close_time), "%Y-%m-%d %H:%M:%S", &ts);
	
	//ctime_s(str, sizeof(str), &time);
	std::cout << "LOG|INFO|RECIEVED: " << trade.symbol << "|" << bar_count << "|" << current_trade_time << "|" << bar_close_time;
	std::cout << "|" << trade.price << "|" << trade.quantity << std::endl;

	/*************************************************************************************/

	//If new bar count is changed than the one that we were updating, then send the older and reset values.
	//First send current state. Then send any empty states
	if (bar_count > state.bar_count && (state.bar_count || (bar_end_time <= seconds_time)))
	{
		//Close current message
		state.close = state.last_trade_price;
		client_messages_thread_safe.push(trade.symbol, state);
		std::cout << "LOG|INFO|CLIENT_MSG: ";
		state.print(std::cout);
		state.increment_bar_count();


		//Send any gaps
		if (send_empty_bars)
		{
			while(state.bar_count < bar_count)
			{
				state.reset();
				client_messages_thread_safe.push(trade.symbol, state);
				std::cout << "LOG|INFO|CLIENT_MSG: ";
				state.print(std::cout);
				state.increment_bar_count();
			}
		}
		//Now start a new state by reseting the values & start tracking new bar
		state.reset();
		state.open = trade.price;
		state.bar_count = bar_count;
	}

	{
		//Just update the state
		state.high = std::max(state.high, trade.price);
		state.low = std::min(state.low?state.low:trade.price, trade.price);
		state.volume += trade.quantity;
		state.last_trade_price = trade.price;
		std::cout << "LOG|INFO|BAR_STATE: " << state.symbol << "|" << state.bar_count << "|" << current_trade_time << "|" << bar_close_time;
		std::cout << "|" << state.open << "|" << state.high << "|" << state.low << "|" << state.close << "|" << state.volume << std::endl;
	}
}

void OHLCServer::handle_new_trade(Trade &trade) 
{
	update_ohlc(trade);
}


