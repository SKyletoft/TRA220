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
	host.resize(dev.size());

	return {std::move(dev), host};
}

__global__
void v3_kernel(
	gpu::device_span2<num> p,
	gpu::device_span2<num> pd,
	gpu::device_span2<num> b,

	// std::span<num> p_,
	// std::span<num> pd_,
	// std::span<num> b_,

	usize nx, usize ny, usize nt,
	num dx2, num dy2
) {
	// auto to_idx = [=](usize y, usize x) -> usize { return y * 64 + x; };
	// auto p = [&](usize y, usize x) -> num& { return p_[y * 64 + x]; };
	// auto pd = [&](usize y, usize x) -> num& { return pd_[y * 64 + x]; };
	// auto b = [&](usize y, usize x) -> num& { return b_[y * 64 + x]; };

	for (auto _i = 0; _i < nt; _i++) {
		for (usize i = 0; i < std::min(pd.size(), p.size()); i++) {
			pd[i] = p[i];
		}

		for (usize i = 1; i < ny - 1; i++) {
			for (usize j = 1; j < nx - 1; j++) {
				p(i, j) = (
						(pd(i, j+1) + pd(i, j-1)) * dy2
						+ (pd(i+1, j) + pd(i-1, j)) * dx2
						- b(i, j) * dx2 * dy2
					)
					/ (2 * (dx2 + dy2));
			}
		}

		for (usize j = 0; j < nx; j++) {
			p(0, j) = 0.;
			p(ny - 1, j) = 0.;
		}
		for (usize i = 0; i < ny; i++) {
			p(i, 0) = 0.;
			p(i, nx - 1) = 0.;
		}
	}
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
	auto to_idx = [=](usize y, usize x) -> usize { return y * 64 + x; };

	num dx = (x_max - x_min) / num(nx - 1);
	num dy = (y_max - y_min) / num(ny - 1);
	num dx2 = dx * dx;
	num dy2 = dy * dy;

	auto [p_, p] = zeroes<num>(nx, ny);
	auto [pd_, pd] = zeroes<num>(nx, ny);
	auto [b_, b] = zeroes<num>(nx, ny);

	assert(p_.size() == p.size() && p_.size() == 64*64);
	assert(p_.pitch() == 64);

	b[to_idx(usize(nx / 4), usize(ny / 4))] = 100.;
	b[to_idx(usize(3 * ny / 4), usize(3 * nx / 4))] = -100.;

	gpu::copy_to_device(std::span{b}, b_);

	v3_kernel<<<1,1>>>(p_, pd_, b_, nx, ny, nt, dx2, dy2);
	// v3_kernel(p, pd, b, nx, ny, nt, dx2, dy2);

	gpu::copy_to_host(std::span{p}, p_);

	std::vector<num> out{};
	for (usize i = 0; i < 50; i++) {
		for (usize j = 0; j < 50; j++) {
			out.push_back(p[i * p_.pitch() + j]);
		}
	}

	return out;
}
