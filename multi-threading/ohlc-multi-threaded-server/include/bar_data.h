#ifndef BAR_DATA_H
#define BAR_DATA_H
#include <string>
#include <iostream>

struct OHLC
{
	std::string symbol;
	double open;
	double high;
	double low;
	double close;
	double volume;
	uint16_t bar_count;
	//Temporarily save until the bar is closed
	double last_trade_price;
	OHLC(std::string sym) noexcept : symbol{ sym }, open{ 0.0 }, high{ 0.0 }, low{ 0.0 },
		close{ 0.0 }, bar_count{ 1 }, volume{ 0.0 },
		last_trade_price{ 0.0 }
	{}
	OHLC() noexcept : OHLC("")
	{}

	inline void increment_bar_count() noexcept
	{
		++bar_count;
	}
	friend std::ostream &operator<<(std::ostream &os, const OHLC &state);

	std::ostream& print(std::ostream& os) noexcept
	{
		os  << "{\"event\": \"ohlc_notify\", \"symbol\" : " << symbol << ", \"bar_num\" :" << bar_count \
			<< ", \"o\" : " << open << " ,\"h\" : " << high << ", \"l\" : " << low << ", \"c\" : " << close\
			<< ", \"volume\" : " << volume << "}" << std::endl;
		return os;
	}
	inline constexpr void reset() noexcept
	{
		open = high = low = close = volume = last_trade_price = 0.0;
		//++bar_count;
	}
};


#endif
