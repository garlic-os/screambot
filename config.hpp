#pragma once
#include <dpp/dpp.h>

#include <cstdint>
#include <string>
#include <vector>

struct Config {
	std::string token;
	std::vector<uint64_t> admin_user_ids;
	std::vector<uint64_t> do_not_reply_user_ids;
	double random_reply_chance_percent;
	uint64_t rate_limit_ms;
};
