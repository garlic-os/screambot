#pragma once
#include <cstdint>
#include <vector>


namespace rng {
	int64_t choose_number(int64_t min, int64_t max);
	template <typename T>
	T choose_element(const std::vector<T>& vec);
	bool chance(double percent);
}
