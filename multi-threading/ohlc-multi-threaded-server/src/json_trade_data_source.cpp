#include <string>
#include "boost/property_tree/ptree.hpp"
#include <boost/property_tree/json_parser.hpp>
#include "../include/json_trade_data_source.h"
#include "../include/thread_safe_map.h"
#include "../include/trade.h"


JSONTradeDataSource::JSONTradeDataSource(MAP_TYPE &data_map, 
	std::string  file_path): stop_flag{false}, shared_data{data_map}, data_file_path { file_path }
{}

void JSONTradeDataSource::run()
{
	std::cout << "LOG|INFO: Starting JSON Data Reader thread" << std::endl;
	// Create empty property tree object
	boost::property_tree::ptree tree;
	boost::property_tree::ptree::value_type data_node;

	try
	{
		boost::property_tree::read_json(data_file_path, tree);
	}
	catch (std::exception &e)
	{
		std::cout << "Caught exception while reading JSON" << e.what() << std::endl;
		std::cout << "Exiting thread" <<  std::endl;
		return;
	}

	//while (!stop_flag)
	{
		int count = 1;
		for (boost::property_tree::ptree::value_type &data_node : tree.get_child("trades"))
		{
			if (stop_flag)
			{
				break;
			}
			//for (boost::property_tree::ptree::value_type &data : data_node.second.get_child(""))
			{
				auto symbol = data_node.second.get<std::string>("sym", "");
				auto quantity = data_node.second.get<double>("Q", 0.0);
				auto price = data_node.second.get<double>("P", 0.0);
				auto side = data_node.second.get<std::string>("side", "");
				auto timestamp = data_node.second.get<uint64_t>("TS2", 0);

				Trade current_trade{ symbol, quantity, side, price, timestamp };

				shared_data.push(symbol, (current_trade));
				++count;
				//std::cout << "LOG|INFO: Published count " << count++ << " Symbol " << current_trade.symbol << std::endl;
			}

		}
		std::cout << "Finished file: Total count " << count << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}
