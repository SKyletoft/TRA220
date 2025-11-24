#include "poisson.hpp"

#include "gpu_2d.hpp"

#include <fmt/base.h>

#include <tuple>
#include <ranges>

#define TEMPLATE_COPYABLE(T) template <typename T> requires std::is_trivially_copyable_v<T>

using i32 = int32_t;
using u32 = uint32_t;
using u64 = uint64_t;
using usize = size_t;
using f32 = float;
using f64 = double;
using num = float;

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
	host.resize(width * height);

	return {std::move(dev), host};
}

__global__
void v3_kernel(
	gpu::device_span2<num> p,
	gpu::device_span2<num> pd,
	gpu::device_span2<num> b,
	num dx2,
	num dy2
) {
	usize i = blockIdx.y * blockDim.y + threadIdx.y;
	usize j = blockIdx.x * blockDim.x + threadIdx.x;

	if (i == 0 || j == 0) return;
	if (i + 1 >= p.height()) return;
	if (j + 1 >= p.width()) return;

	p(i, j) = (
			(pd(i, j+1) + pd(i, j-1)) * dy2
			+ (pd(i+1, j) + pd(i-1, j)) * dx2
			- b(i, j) * dx2 * dy2
		)
		/ (2 * (dx2 + dy2));

}

auto poisson(
	usize nx,
	usize ny,
	usize nt,
	num x_min,
	num x_max,
	num y_min,
	num y_max
) -> std::vector<num> {
	num dx = (x_max - x_min) / num(nx - 1);
	num dy = (y_max - y_min) / num(ny - 1);
	num dx2 = dx * dx;
	num dy2 = dy * dy;

	auto [p_, p] = zeroes<num>(nx, ny);
	auto [pd_, pd] = zeroes<num>(nx, ny);
	auto [b_, b] = zeroes<num>(nx, ny);

	b[b_.get_index(usize(nx / 4), usize(ny / 4))] = 100.;
	b[b_.get_index(usize(3 * ny / 4), usize(3 * nx / 4))] = -100.;

	gpu::copy_to_device(std::span{b}, b_);

	for (auto _ : v::iota(0uz, nt)) {
		gpu::memcpy(pd_, p_);

		v3_kernel<<<dim3(5,5,1),dim3(16, 16, 1)>>>(
			p_, pd_, b_,
			dx2, dy2
		);

		gpu::device_memset(p_.subspan2(0,              0,               p_.width(),  1),           0.f);
		gpu::device_memset(p_.subspan2(0,              p_.height() - 1, p_.width(),  1),           0.f);
		gpu::device_memset(p_.subspan2(0,              0,               1,           p_.height()), 0.f);
		gpu::device_memset(p_.subspan2(p_.width() - 1, 0,               1,           p_.height()), 0.f);
	}

	gpu::copy_to_host(std::span{p}, p_);

	return p;
}
