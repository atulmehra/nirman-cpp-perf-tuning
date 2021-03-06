// OHLCServer.cpp : Defines the entry point for the console application.
//

#include <string>
#include <filesystem>
#include "include/ohlc_server.h"

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		std::cout << "LOG|ERROR: Provide 2 arguments." << std::endl;
		std::cout << "LOG|HELP: Usage - OHLCServer.exe <Absolute Path JSON with Trades> <Absolue Path JSON with Subscriptions" << std::endl;
		return -1;
	}

	std::string trades_file_path = argv[1];
	std::string subscription_file_path = argv[2];
	bool send_empty_bars = (argc > 3 && std::string(argv[3]) == "SendEmptyBars") ? true : false;

	std::cout << "LOG|INFO: Output File Path = .\\Output.text. Input files: " << trades_file_path << " "
			<< subscription_file_path << " Send Empty bars = " << (send_empty_bars? "True" : "False") << std::endl;
	try
	{
		if ((std::filesystem::exists(trades_file_path) && std::filesystem::exists(subscription_file_path)))
		{
		}
	}
	catch (std::exception &e)
	{
		std::cout << "LOG|ERROR: Either of the file paths provided doesn't exist. Exiting" << e.what() << std::endl;
		return -2;
	}

	auto ohlcserver = std::make_shared<OHLCServer>(trades_file_path, send_empty_bars);
	std::cout << "LOG|NOTICE: Press any key to start processing" << std::endl;
	std::cout << "LOG|NOTICE: Once running, you can run any key again to stop..." << std::endl;
	getchar();
	ohlcserver->get_client_activity()->init(ohlcserver, subscription_file_path);
	ohlcserver->init(subscription_file_path);
	std::cout << "LOG|INFO: Initialized thread" << std::endl;
	std::cout << "LOG|INFO: Will stop threads when user presses key" << std::endl;
	getchar();
	std::cout << "*********** User pressed key to stop ************" << std::endl;
	ohlcserver->stop();
	std::cout << "LOG|NOTICE: Stopped the server and threads. Exiting now" << std::endl;

    return 0;
}

