#include "poisson.hpp"

#include "gpu_2d.hpp"

#include <tuple>
#include <ranges>

#define TEMPLATE_COPYABLE(T) template <typename T> requires std::is_trivially_copyable_v<T>

using u32 = uint32_t;

namespace v = std::views;

TEMPLATE_COPYABLE(T)
auto zeroes(
	size_t width,
	size_t height
) -> std::tuple<
	gpu::device_unique_ptr2<T>,
	std::vector<T>
> {
	gpu::device_unique_ptr2<T> dev { width, height };
	gpu::device_memset(dev, {0});
	std::vector<T> host{};
	host.resize(dev.size());

	return {std::move(dev), host};
}

__global__ void calculate_cell(
	gpu::device_span2<float> pd,
	gpu::device_span2<float> p,
	gpu::device_span2<float> b,
	float dx2,
	float dy2
) {
	size_t i = blockIdx.y * blockDim.y + threadIdx.y;
	size_t j = blockIdx.x * blockDim.x + threadIdx.x;

	if (p.in_bounds(i, j)) {
		p[i, j] = (
				(pd[i, j+1] + pd[i, j-1]) * dy2
				+ (pd[i+1, j] + pd[i-1, j]) * dx2
				- b[i, j] * dx2 * dy2
			)
			/ (2 * (dx2 + dy2));
	}
}

auto poisson(
	size_t nx,
	size_t ny,
	size_t nt,
	float x_min,
	float x_max,
	float y_min,
	float y_max
) -> std::vector<float> {
	float dx = (x_max - x_min) / float(nx - 1);
	float dy = (y_max - y_min) / float(ny - 1);

	auto [p, p_] = zeroes<float>(nx, ny);
	auto [pd, pd_] = zeroes<float>(nx, ny);
	auto [b, b_] = zeroes<float>(nx, ny);

	b_[b.get_index(ny / 4, nx / 4)] = 100.f;
	b_[b.get_index(3 * ny / 4, 3 * nx / 4)] = -100.f;
	gpu::copy_to_device(std::span{b_}, b);

	for (auto _ : v::iota(0uz, nt)) {
		gpu::memcpy(pd, p);

		dim3 block_size(16, 16);
		dim3 grid_size(((u32)nx + block_size.x - 1) / block_size.x,
			       ((u32)ny + block_size.y - 1) / block_size.y);
		calculate_cell<<<grid_size, block_size>>>(pd, p, b, dx*dx, dy*dy);

		gpu::device_memset(p.subspan2(0,             0,              p.width(),  1),          0.f);
		gpu::device_memset(p.subspan2(0,             p.height() - 1, p.width(),  1),          0.f);
		gpu::device_memset(p.subspan2(0,             0,              1,          p.height()), 0.f);
		gpu::device_memset(p.subspan2(p.width() - 1, 0,              1,          p.height()), 0.f);
	}

	gpu::copy_to_host(std::span{p_}, p);
	return p_;
}
