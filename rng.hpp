#pragma once
#include <cstdint>
#include <random>
#include <vector>

namespace rng {
	inline int64_t choose_number(int64_t min, int64_t max) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(min, max);
		return dis(gen);
	}

	template <typename T>
	T choose_element(const std::vector<T>& vec) {
		return vec[choose_number(0, vec.size() - 1)];
	}

	inline bool chance(double percent) {
		return choose_number(0, 100) < percent;
	}
}
