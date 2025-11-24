#include "poisson.hpp"

#include <fmt/base.h>
#include <fmt/ranges.h>

#include <stdexcept>

auto main() -> int {
	try {
		// What does any of this mean? Who knows
		auto res = poisson(50, 50, 100, 0, 2, 0, 1);
#ifdef DIFF
		for (auto x : res) {
			fmt::println("{:.05e}", x);
		}
#else
		for (std::size_t j = 0; j < 50; ++j) {
			for (std::size_t i = 0; i < 50; ++i) {
				auto z = res[j * 50 + i];
				fmt::println("{}\t{}\t{}", i, j, z);
			}
		}
#endif
	}
	catch (std::runtime_error e) {
		fmt::println("Uncaught exception: {}", e.what());
	}
}
