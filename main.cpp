
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <dpp/dpp.h>
#include "./config.hpp"
#include "./screambot.hpp"

using json = dpp::json;


int main(int argc, char* argv[]) {
	try {
		Config config;
		{
			std::string config_path = argc > 1 ? argv[1] : "config.json";
			std::ifstream fin(config_path);
			json data = json::parse(fin);
			data.at("token").get_to(config.token);
			data.at("admin_user_ids").get_to(config.admin_user_ids);
			data.at("do_not_reply_user_ids").get_to(config.do_not_reply_user_ids);
			data.at("random_reply_chance_percent").get_to(config.random_reply_chance_percent);
			data.at("rate_limit_ms").get_to(config.rate_limit_ms);
		}

		Screambot screambot(&config);
		screambot.start();

	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 2;
	} catch (...) {
		std::cerr << "Unknown exception" << std::endl;
		return 3;
	}
	return 0;
}
