#ifndef JSON_DATA_READER_SOURCE_H
#define JSON_DATA_READER_SOURCE_H

#include <chrono>
#include <iostream>
#include <thread>
#include <queue>
#include "trade.h"
//#include "trade_time_up_message.h"

template <typename KEY, typename ELEMENT_TYPE>
class ThreadSafeMap;

struct Trade;

//template <typename KEY, typename ELEMENT_TYPE>
class JSONTradeDataSource
{
public:
	typedef ThreadSafeMap<std::string, Trade> MAP_TYPE;
	JSONTradeDataSource(MAP_TYPE &data_map, std::string filename);

	//Non copyable, non assignable
	JSONTradeDataSource(const JSONTradeDataSource &) = delete;
	JSONTradeDataSource(JSONTradeDataSource &&) = delete;
	JSONTradeDataSource& operator = (const JSONTradeDataSource &) = delete;
	JSONTradeDataSource& operator = (JSONTradeDataSource &&) = delete;

	inline void stop()
	{
		stop_flag = true;
	}
	void run();
protected:
	void read_record(Trade &trade);
	MAP_TYPE & shared_data;
	volatile bool stop_flag;
	std::string data_file_path;
//	typename boost::asio::io_context data_source_thread_context;;
};

#endif
