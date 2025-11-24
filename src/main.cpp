#include "poisson.hpp"

#include <fmt/base.h>
#include <fmt/ranges.h>

#undef NDEBUG

#include <stdexcept>
#include <iostream>
#include <cassert>

static constexpr size_t width = 50;
static constexpr size_t height = 50;

// #define DIFF

auto main() -> int {
	// What does any of this mean? Who knows
	auto res = poisson(width, height, 100, 0, 2, 0, 1);
#ifdef DIFF
	for (auto x : res) {
		fmt::println("{:.05e}", x);
	}
#else
	fmt::println("{}\t{}\t0", width, height);
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			auto z = res[j * width + i];
			fmt::println("{}\t{}\t{}", i, j, z);
		}
	}
#endif
}
