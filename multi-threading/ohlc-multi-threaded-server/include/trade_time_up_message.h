#ifndef TRADE_TIME_UP_MESSAGE_H
#define OHLC_SERVER_H
#include <boost/asio.hpp>
#include <chrono>

class TradeTimeUpMessage
{
public:
	//Callback with bar_num and security
	typedef std::function<void(int, std::string)> CALLABLE;
	TradeTimeUpMessage(boost::asio::io_context& data_source_context, std::function<void(int, std::string)> callback,
		int seconds, int bar_num, std::string symbol) :
		timer_{io, boost::asio::chrono::seconds(seconds)}, callable{ callback }, timer_duration_seconds{seconds}
	{
	}

	void start()
	{
		timer_.async_wait(std::bind(callable, bar_num, symbol));
	}
	void add_next_event()
	{
		timer_.expires_at(std::chrono::steady_clock::now() + boost::asio::chrono::seconds(timer_duration_seconds));
		timer_.async_wait(callable);
	}

private:
	boost::asio::steady_timer timer_;
	std::string symbol;
	int bar_num;
	int timer_duration_seconds;
	CALLABLE callable;
};

#endif
