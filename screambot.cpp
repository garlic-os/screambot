#include "./screambot.hpp"

#include <dpp/dpp.h>

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <utility>

#include "./rng.hpp"

using system_clock = std::chrono::system_clock;

bool mentions_user(const dpp::message &msg, const dpp::snowflake &user_id) {
	for (const auto &mention : msg.mentions) {
		if (std::get<dpp::user>(mention).id == user_id) {
			return true;
		}
	}
	return false;
}

bool contains_scream(const dpp::message &msg) {
	return boost::algorithm::to_lower_copy(msg.content).find("aaa") !=
		   std::string::npos;
}

std::string tag(const dpp::user &user) {
	if (user.discriminator == 0) {
		return "@" + user.username;
	}
	return "@" + user.format_username();
}

std::string multiply_string(uint64_t n, const std::string &str) {
	std::stringstream out;
	while (n--) {
		out << str;
	}
	return out.str();
}

std::vector<time_t> *get_or_create(
	std::unordered_map<dpp::snowflake, std::vector<time_t> > &map,
	const dpp::snowflake &key
) {
	try {
		return &(map.at(key));
	} catch (std::out_of_range &e) {
		map[key] = {};
		return &(map[key]);
	}
}

Screambot::Screambot(const Config *config) {
	m_config = config;

	auto intents =
		dpp::intents::i_default_intents | dpp::intents::i_message_content;
	m_client = new dpp::cluster(m_config->token, intents);
	m_client->on_log(dpp::utility::cout_logger());

	m_client->on_ready([this](const dpp::ready_t & /*event*/) {
		std::cout << "Logged in as " << tag(m_client->me) << std::endl;
		m_client->set_presence(dpp::presence(
			dpp::presence_status::ps_online,
			dpp::activity_type::at_game,
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
		));
	});

	m_client->on_message_create([this](const dpp::message_create_t event) {
		if (event.msg.author.id == m_client->me.id) {
			return;
		}
		if (try_command(event)) {
			return;
		}
		if (in_do_not_reply(event.msg.author.id)) {
			return;
		}
		if (mentions_user(event.msg, m_client->me.id)) {
			std::cout << "Pung by " << tag(event.msg.author) << std::endl;
			scream(event.msg.channel_id);
			return;
		}
		if (contains_scream(event.msg)) {
			std::cout << "Screamed at by " << tag(event.msg.author)
					  << std::endl;
			scream(event.msg.channel_id);
			return;
		}
		if (event.msg.is_dm()) {
			std::cout << "Received a DM from " << tag(event.msg.author) << ": "
					  << event.msg.content << std::endl;
			scream(event.msg.channel_id);
			return;
		}
		if (random_reply_chance(event.msg.channel_id)) {
			std::cout << "Randomly decided to scream" << std::endl;
			scream(event.msg.channel_id);
			return;
		}
		log_received_message(event.msg);
	});
}

Screambot::~Screambot() { delete m_client; }

void Screambot::start() { m_client->start(dpp::start_type::st_wait); }

void Screambot::scream(
	const dpp::snowflake &channel_id,
	bool bypass_rate_limit
) {
	if (rate_limited(channel_id) && !bypass_rate_limit) {
		std::cout << "- Failed to scream: rate limited" << std::endl;
		return;
	}
	m_client->message_create(dpp::message(channel_id, generate_scream()));
	log_sent_message(channel_id);
}

bool Screambot::is_admin(const dpp::snowflake &user_id) const {
	return std::find(
			   m_config->admin_user_ids.begin(),
			   m_config->admin_user_ids.end(),
			   user_id
		   ) != m_config->admin_user_ids.end();
}

bool Screambot::in_do_not_reply(const dpp::snowflake &user_id) const {
	return std::find(
			   m_config->do_not_reply_user_ids.begin(),
			   m_config->do_not_reply_user_ids.end(),
			   user_id
		   ) != m_config->do_not_reply_user_ids.end();
};

bool Screambot::rate_limited(const dpp::snowflake &channel_id) const {
	if (!m_sent_activity_log.contains(channel_id)) {
		return false;
	}
	time_t now = system_clock::to_time_t(system_clock::now());
	time_t last_message_time = m_sent_activity_log.at(channel_id);
	uint64_t duration =
		now - last_message_time;  // Unsigned because now is always greater
	return duration * 1000 < m_config->rate_limit_ms;
};

std::string Screambot::generate_scream() const {
	uint64_t body_length = rng::choose_number(1, 100);

	// Vanilla scream half the time
	if (rng::chance(50)) {
		return std::string(body_length, 'A');
	}

	static std::vector<std::string> body_choices = {"A", "O"};
	std::string body =
		multiply_string(body_length, rng::choose_element(body_choices));

	// Chance to wrap the message in one of these Markdown strings
	static std::vector<std::string> formatter_choices = {"*", "**", "***"};
	std::string formatter =
		rng::chance(50) ? "" : rng::choose_element(formatter_choices);

	// Chance to put one of these at the end of the message
	static std::vector<std::string> suffix_choices = {"H", "RGH", "ER"};
	std::string suffix =
		rng::chance(50) ? "" : rng::choose_element(suffix_choices);

	// Chance to add exclamation points
	std::string punctuation =
		rng::chance(50) ? "" : multiply_string(rng::choose_number(0, 5), "!");

	std::string result = formatter + body + suffix + punctuation + formatter;

	// Chance for lowercase
	if (rng::chance(12.5)) {
		boost::algorithm::to_lower(result);
	}

	return result;
}

bool Screambot::try_command(const dpp::message_create_t &event) {
	if (!event.msg.content.starts_with("!screambot")) {
		return false;
	}
	std::vector<std::string> args =
		dpp::utility::tokenize(event.msg.content, " ");

	if (args[1] == "info" || args[1] == "help" || args[1] == "invite") {
		event.send(
			generate_scream() +
			"\n"
			"CODE: https://github.com/garlic-os/screambot-plus-plus\n"
			"INVITE: "
			"https://discord.com/api/oauth2/"
			"authorize?client_id=574092583014236160&permissions=274877910016&"
			"scope=bot\n"
		);
		return true;
	}

	if (!is_admin(event.msg.author.id)) {
		return false;
	}
	if (args[1] == "scream") {
		if (args.size() != 3) {
			event.send(
				"AAAAAAAAAAAAA USAGE: !screambot scream <channel_id> "
				"AAAAAAAAAAAAAAAAAAA"
			);
			return true;
		}
		dpp::snowflake channel_id = args[2];
		scream(channel_id, true);
		return true;
	}
	if (args[1] == "say") {
		if (args.size() < 4) {
			event.send(
				"AAAAAAAAAAAAA USAGE: !screambot say <channel_id> <message> "
				"AAAAAAAAAAAAAAAAAAA"
			);
			return true;
		}
		dpp::snowflake channel_id = args[2];
		std::string message = boost::algorithm::join(
			std::vector<std::string>(args.begin() + 3, args.end()), " "
		);
		m_client->message_create(dpp::message(channel_id, message));
		return true;
	}
	return false;
}

// Decide to reply more often when there has been more activity in the channel.
// https://www.desmos.com/calculator/49qgowmiun
bool Screambot::random_reply_chance(const dpp::snowflake &channel_id) const {
	size_t activity_level = 0;
	if (m_received_activity_log.contains(channel_id)) {
		activity_level = m_received_activity_log.at(channel_id).size();
	}
	const double a = 2.55;
	const uint64_t b = 190;
	double reply_chance = std::min(std::pow(activity_level, a) / b, 50.0);
	return rng::chance(reply_chance);
}

void Screambot::log_received_message(const dpp::message &message) {
	dpp::snowflake channel_id = message.channel_id;
	time_t timestamp = message.sent;
	std::vector<time_t> *channel_log =
		get_or_create(m_received_activity_log, channel_id);
	while (channel_log->size() > 0 && timestamp - channel_log->front() > 10) {
		// Only keep entries from the last 10 seconds
		channel_log->erase(channel_log->begin());
	}
	channel_log->push_back(timestamp);
}

void Screambot::log_sent_message(const dpp::snowflake &channel_id) {
	const auto now = system_clock::now();
	const std::time_t timestamp = system_clock::to_time_t(now);
	m_sent_activity_log[channel_id] = timestamp;
}
