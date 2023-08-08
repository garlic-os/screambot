#include <cstdint>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <dpp/dpp.h>
#include "./screambot.hpp"
#include "./rng.hpp"


bool mentions_user(const dpp::message& msg, dpp::snowflake user_id) {
	for (const auto& mention : msg.mentions) {
		if (std::get<dpp::user>(mention).id == user_id) {
			return true;
		}
	}
	return false;
}


bool contains_scream(const dpp::message& msg) {
	return boost::algorithm::to_lower_copy(msg.content).find("aaa")
		!= std::string::npos;
}


std::string multiply_string(uint64_t n, const std::string& str) {
	std::stringstream out;
	while (n--) {
		out << str;
	}
	return out.str();
}


Screambot::Screambot(const Config* config) {
	m_config = config;

	auto intents = dpp::intents::i_default_intents |
				   dpp::intents::i_message_content;
	m_client = new dpp::cluster(m_config->token, intents);
	m_client->on_log(dpp::utility::cout_logger());

	m_client->on_ready([this](const dpp::ready_t& /*event*/) {
		std::cout << "Logged in as " << m_client->me.format_username()
				  << std::endl;
		m_client->set_presence(
			dpp::presence(
				dpp::presence_status::ps_online,
				dpp::activity_type::at_game,
				"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			)
		);
	});

	m_client->on_message_create([this](const dpp::message_create_t event) {
		if (try_command(event)) {
			return;
		}
		if (in_do_not_reply(event.msg.author.id)) {
			return;
		}
		if (mentions_user(event.msg, m_client->me.id)) {
			std::cout << "Pung" << std::endl;
			scream(event.msg.channel_id);
			return;
		}
		if (contains_scream(event.msg)) {
			std::cout << "Screamed at" << std::endl;
			scream(event.msg.channel_id);
			return;
		}
		if (event.msg.is_dm()) {
			std::cout << "Received a DM from " << event.msg.author.format_username()
					  << ": " << event.msg.content << std::endl;
			scream(event.msg.channel_id);
			return;
		}
		if (rng::chance(m_config->random_reply_chance_percent)) {
			std::cout << "Randomly decided to scream" << std::endl;
			scream(event.msg.channel_id);
			return;
		}
	});
}


Screambot::~Screambot() {
	delete m_client;
}


void Screambot::start() {
	m_client->start(dpp::start_type::st_wait);
}


void Screambot::scream(dpp::snowflake channel_id, bool bypass_rate_limit) {
	if (rate_limited(channel_id) && !bypass_rate_limit) {
		std::cout << "- Failed to scream: rate limited" << std::endl;
		return;
	}
	m_client->message_create(
		dpp::message(channel_id, generate_scream())
	);
	m_last_message_times[channel_id] = std::chrono::system_clock::now();
}


bool Screambot::is_admin(dpp::snowflake user_id) const {
	return std::find(
		m_config->admin_user_ids.begin(),
		m_config->admin_user_ids.end(),
		user_id
	) != m_config->admin_user_ids.end();
}


bool Screambot::in_do_not_reply(dpp::snowflake user_id) const {
	return std::find(
		m_config->do_not_reply_user_ids.begin(),
		m_config->do_not_reply_user_ids.end(),
		user_id
	) != m_config->do_not_reply_user_ids.end();
};


bool Screambot::rate_limited(dpp::snowflake channel_id) const {
	// See if less than m_config->rate_limit_ms has passed since the last
	// time Screambot screamed in this channel
	
};


std::string Screambot::generate_scream() const {
	uint64_t body_length = rng::choose_number(1, 100);

	// Vanilla scream half the time
	if (rng::chance(50)) {
		return std::string(body_length, 'A');
	}

	static std::vector<std::string> body_choices = { "A", "O" };
	std::string body = multiply_string(body_length, rng::choose_element(body_choices));

	// Chance to wrap the message in one of these Markdown strings
	static std::vector<std::string> formatter_choices = { "*", "**", "***" };
	std::string formatter = rng::chance(50)
		? ""
		: rng::choose_element(formatter_choices);

	// Chance to put one of these at the end of the message
	static std::vector<std::string> suffix_choices = { "H", "RGH", "ER" };
	std::string suffix = rng::chance(50)
		? ""
		: rng::choose_element(suffix_choices);

	// Chance to add exclamation points
	std::string punctuation = rng::chance(50)
		? ""
		: multiply_string(rng::choose_number(0, 5), "!");

	std::string result = formatter + body + suffix + punctuation + formatter;

	// Chance for lowercase
	if (rng::chance(12.5)) {
		boost::algorithm::to_lower(result);
	}

	return result;
}


bool Screambot::try_command(const dpp::message_create_t& event) {
	if (!is_admin(event.msg.author.id)) {
		return false;
	}
	if (!event.msg.content.starts_with("!screambot")) {
		return false;
	}
	std::vector<std::string> args = dpp::utility::tokenize(event.msg.content, " ");
	if (args[1] == "scream") {
		if (args.size() < 3) {
			event.send("Usage: !screambot scream <channel_id>");
			return true;
		}
		dpp::snowflake channel_id = args[2];
		scream(channel_id, true);
		return true;
	}
	if (args[1] == "say") {
		if (args.size() < 4) {
			event.send("Usage: !screambot say <channel_id> <message>");
			return true;
		}
		dpp::snowflake channel_id = args[2];
		std::string message = boost::algorithm::join(
			std::vector<std::string>(args.begin() + 3, args.end()),
			" "
		);
		m_client->message_create(
			dpp::message(channel_id, message)
		);
		return true;
	}
	return false;
}
