#ifndef TRADE_H
#define TRADE_H

#include <string>
struct Trade
{
	std::string symbol;
	double quantity;
	std::string side;
	double price;
	uint64_t timestamp;
};

#endif
