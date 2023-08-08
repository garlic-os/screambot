#pragma once
#include <chrono>
#include <dpp/dpp.h>
#include "./config.hpp"


class Screambot {
  private:
	const Config* m_config;
	dpp::cluster* m_client;
	std::unordered_map<
		dpp::snowflake,
		std::chrono::time_point<std::chrono::system_clock>
	> m_last_message_times;
  public:
	explicit Screambot(const Config* config);
	virtual ~Screambot();

	void start();
	void scream(dpp::snowflake channel_id, bool bypass_rate_limit=false);
	std::string generate_scream() const;
	bool try_command(const dpp::message_create_t& event);
	bool is_admin(dpp::snowflake user_id) const;
	bool in_do_not_reply(dpp::snowflake user_id) const;
	bool rate_limited(dpp::snowflake channel_id) const;
};
