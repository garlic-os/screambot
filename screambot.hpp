#pragma once
#include <dpp/dpp.h>

#include <ctime>
#include <vector>

#include "./config.hpp"

class Screambot {
   private:
	const Config *m_config;
	dpp::cluster *m_client;
	std::unordered_map<dpp::snowflake, std::vector<time_t> >
		m_received_activity_log;
	std::unordered_map<dpp::snowflake, time_t> m_sent_activity_log;

   public:
	explicit Screambot(const Config *config);
	virtual ~Screambot();

	void start();
	void
	scream(const dpp::snowflake &channel_id, bool bypass_rate_limit = false);
	void log_received_message(const dpp::message &message);
	void log_sent_message(const dpp::snowflake &channel_id);
	std::string generate_scream() const;
	bool try_command(const dpp::message_create_t &event);
	bool is_admin(const dpp::snowflake &user_id) const;
	bool in_do_not_reply(const dpp::snowflake &user_id) const;
	bool rate_limited(const dpp::snowflake &channel_id) const;
	bool random_reply_chance(const dpp::snowflake &channel_id) const;
};
