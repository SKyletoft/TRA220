#include "poisson.hpp"
// #include "poisson_3d.hpp"

#include <fmt/base.h>
#include <fmt/ranges.h>

#undef NDEBUG

#include <stdexcept>
#include <iostream>
#include <cassert>

static constexpr size_t width3d = 10;
static constexpr size_t width   = 50;
static constexpr size_t height  = 50;

void run_2d() {
	// What does any of this mean? Who knows
	auto res = poisson(width, height, 100, 0, 2, 0, 1);
	fmt::println("{}\t{}\t0", height, width);
	for (size_t j = 0; j < height; ++j) {
		for (size_t i = 0; i < width; ++i) {
			auto z = res[j * width + i];
			fmt::println("{}\t{}\t{}", i, j, z);
		}
	}
}

/* void run_3d() {
	auto res = poisson_3d<float>(width3d, 100);
	fmt::println("{}\t{}\t{}", width3d, width3d, width3d);
	for (size_t k = 0; k < width3d; ++k) {
		for (size_t j = 0; j < width3d; ++j) {
			for (size_t i = 0; i < width3d; ++i) {
				auto z = res[k * width3d * width3d + j * width3d + i];
				fmt::println("{}\t{}\t{}\t{}", i, j, k, z);
			}
		}
	}
} */

auto main() -> int {
	run_2d();
}
