#include "gpu.hpp"

#include <fmt/base.h>
#include <fmt/ranges.h>

#include <span>
#include <vector>

__global__ void set_index(gpu::device_span<int> mem) {
	mem[threadIdx.x] = (int) (threadIdx.x);
}

auto main() -> int {
	std::vector<int> v{};
	v.resize(12uz);
	gpu::device_unique_ptr<int> p{12uz};
	gpu::device_memset(p, 0);

	set_index<<<1, 12>>>(p);

	gpu::copy_to_host(std::span{v}, p);

	fmt::println("Hello world!\n");
	fmt::println("{}\n", v);
}
