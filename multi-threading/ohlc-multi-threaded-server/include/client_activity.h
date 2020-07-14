#ifndef CLIENT_ACTIVITY_H
#define CLIENT_ACTIVITY_H
#include <string>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include "bar_data.h"
#include "activity_base.h"
#include "boost/property_tree/ptree.hpp"
#include <boost/property_tree/json_parser.hpp>

class OHLCServer;

template <typename CALLABLE>
class ClientActivity : public ActivityBase <std::string, OHLC>
{
public:
	ClientActivity(MAP_TYPE &data_map, CALLABLE function) : ActivityBase{ data_map },
		callback{ function }
	{
		//client_subscriptions["XZECXXBT"].insert(1);
		//client_subscriptions["XETHZUSD"].insert(2);
		//client_subscriptions["XXLMXXBT"].insert(2);
		
	}
	void handle_client_message(OHLC& bar_data) noexcept;
	/*inline void add_subscription(uint32_t id, std::string symbol) noexcept
	{
		client_subscriptions[id].second.insert[symbol];
	}*/

	void read_subscriptions(const std::string& subscription_file_path);

	void init(std::weak_ptr<OHLCServer> ohlc_server, std::string subscriptions_file_path) noexcept;

	virtual void run() override
	{
		std::cout << "LOG|INFO: Starting Client Activity thread" << std::endl;
		while (!stop_flag)
		{
			for (const auto& client_subscription : client_subscriptions)
			{
				if (stop_flag)
					break;
				
				while (auto client_notif_ohlc = shared_data.try_pop(std::string(client_subscription.first)))
				{
					{
						//Todo::This code needs to avoid starvation for some subscription
						callback(*client_notif_ohlc);
					}
				}
			}
		}
	}

	~ClientActivity()
	{
		ofs.flush();
		ofs.close();
	}
private:
	CALLABLE callback;
	std::weak_ptr<OHLCServer> ohlc_server;

	typedef uint32_t CLIENT_ID;
	typedef std::string SYMBOL;

	typedef std::unordered_map<SYMBOL, std::unordered_set<CLIENT_ID> > CLIENT_SUBSCR_MAP;
	CLIENT_SUBSCR_MAP client_subscriptions;
	//Todo: Maintain a list of multi index map where elements are also
	//maintained by client id. Thus when a client unsubscribes the subscriptions can be removed.

	//Out file
	std::ofstream ofs;
};

template <typename CALLABLE>
void ClientActivity<CALLABLE>::read_subscriptions(const std::string& subscription_file_path)
{
	boost::property_tree::ptree tree;
	boost::property_tree::ptree::value_type data_node;

	try
	{
		boost::property_tree::read_json(subscription_file_path, tree);		                                           
		for (auto &data_node : tree.get_child("subscriptions"))
		{
			auto client_id = data_node.second.get<uint32_t>("client_id");
			std::cout << "LOG|INFO: Client " << client_id << " Add Subscriptions. Symbols =  ";
			for (auto &symbol_data : data_node.second)
			{
				if (symbol_data.first == "sym")
				{
					for (auto &current_value : symbol_data.second)
					{
						auto symbol = current_value.second.data();
						client_subscriptions[symbol].insert(client_id);
						std::cout << " " << symbol;
					}
					std::cout << std::endl;
				}
			}

		}
	}
	catch (std::exception &e)
	{
		std::cout << "Caught exception while reading JSON for subscription" << e.what() << std::endl;
		std::cout << "Exiting thread" << std::endl;
		return;
	}

}

template <typename CALLABLE>
void ClientActivity<CALLABLE>::init(std::weak_ptr<OHLCServer> ohlc_server, std::string subscriptions_file_path) noexcept
{
	this->ohlc_server = ohlc_server;
	read_subscriptions(subscriptions_file_path);
	ofs.open("Output.txt", std::ofstream::out);

	for (const auto& client_subscription : client_subscriptions)
	{
		//for (const auto & subscription_symbol : client_subscription.second)
		{
			try
			{
				if (auto shared_ohlcserver = this->ohlc_server.lock())
				{
					shared_ohlcserver->add_subscription(client_subscription.first);
					std::cout << "LOG|INFO: Server - Add Subscription. Symbol = " << client_subscription.first << std::endl;
				}
				else
				{
					std::cout << "LOG|ERROR: while adding subscription for Symbol" << client_subscription.first << std::endl;
					std::cout << "LOG|ERROR: Server thread/instance is not running" << std::endl;
				}
			}
			catch (std::exception &e)
			{
				std::cout << "LOG|ERROR: while adding subscription for Symbol" << client_subscription.first << " " << e.what() << std::endl;
	
			}
		}
	}
}

template <typename CALLABLE>
void ClientActivity<CALLABLE>::handle_client_message(OHLC & bar_data) noexcept
{
	for (auto &client_id : client_subscriptions[bar_data.symbol])
	{
		ofs << "LOG|INFO|CLIENT_MSG: " << client_id << "|";
		bar_data.print(ofs);
		ofs.flush();
		//bar_data.print(std::cout);
	}
}

#endif
