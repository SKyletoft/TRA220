#include "poisson.hpp"

#include <fmt/base.h>
#include <fmt/ranges.h>

#include <stdexcept>

auto main() -> int {
	try {
		// What does any of this mean? Who knows
		auto res = poisson(50, 50, 100, 0, 2, 0, 1);
		fmt::println("{}\n", res);
	}
	catch (std::runtime_error e) {
		fmt::println("Uncaught exception: {}", e.what());
	}
}
