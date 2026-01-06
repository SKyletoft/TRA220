#pragma once

#include "gpu_3d.hpp"

#include <fmt/base.h>
#include <fmt/ranges.h>

#include <ranges>

#define TEMPLATE_COPYABLE(T)                                                                     \
	template <typename T>                                                                    \
		requires std::is_trivially_copyable_v<T>

#define TEMPLATE_COPYABLE_MATHS(T)                                                                \
	template <typename T>                                                                     \
		requires std::is_trivially_copyable_v<T>                                          \
			&& std::assignable_from<T&, T>                                            \
			&& requires(T a, T b) {                                                   \
				{ a + b } -> std::same_as<T>;                                     \
				{ a * b } -> std::same_as<T>;                                     \
			}


#if __cplusplus < 202600L
#define pre(_)
#define post(_)
#endif

using i32   = int32_t;
using u32   = uint32_t;
using u64   = uint64_t;
using usize = size_t;
using f32   = float;
using f64   = double;

namespace v = std::views;

static constexpr size_t WIDTH = 50;
static constexpr size_t HEIGHT = 50;
static constexpr size_t DEPTH = 50;

TEMPLATE_COPYABLE_MATHS(T)
__global__ void matrix_mul(
	gpu::device_span2<T> l,
	gpu::device_span2<T> r,
	gpu::device_span2<T> out
) {
	size_t x = threadIdx.x + blockIdx.x * blockDim.x;
	size_t y = threadIdx.y + blockIdx.y * blockDim.y;

	if (
		y >= out.width()
		|| x >= out.height()
		|| out.width() != out.height()
	) {
		return;
	}

	T sum = {0};

	for (size_t k = 0; k < l.width(); ++k) {
		sum += l(y, k) * r(k, x);
	}

	out(y, x) = sum;
}

TEMPLATE_COPYABLE_MATHS(T)
__global__ void matrix_add(
	gpu::device_span2<T> l,
	gpu::device_span2<T> r,
	gpu::device_span2<T> out
) {
	size_t x = threadIdx.x + blockIdx.x * blockDim.x;
	size_t y = threadIdx.y + blockIdx.y * blockDim.y;

	if (
		y >= out.width()
		|| x >= out.height()
		|| out.width() != out.height()
	) {
		return;
	}

	T sum = {0};

	for (size_t k = 0; k < l.width(); ++k) {
		sum += l(y, k) * r(k, x);
	}

	out(y, x) = sum;
}

template <int x, int y, typename T>
using CpuMatrix = std::array<std::array<T, x>, y>;

template <int x, int y, typename T>
	requires std::is_trivially_copyable_v<T>
class Matrix {
	gpu::device_unique_ptr2<T> inner;

public:
	Matrix()
		: inner(x, y)
	{}

	Matrix(T t)
		: inner(x, y)
	{
		gpu::device_memset(this->inner, std::bit_cast<i32>(t));
	}

	Matrix(std::initializer_list<T> fields)
		: inner(x, y)
		pre(fields.size() != x * y)
	{
		if (fields.size() != x * y) {
			throw std::invalid_argument("Wrong amount of arguments in Matrix constructor");
		}
		std::span<const T> fields_as_span {fields.begin(), fields.size()};
		gpu::copy_to_device(
			fields_as_span,
			inner
		);
	}

	Matrix(std::array<T, x * y> arr)
		: inner(x, y)
	{
		gpu::copy_to_device(std::span(arr), inner);
	}

	Matrix operator*(Matrix const& rhs) {
		Matrix out{};
		matrix_mul<<<1, dim3(x, y, 1)>>>(this->inner, rhs.inner, out.inner);
		return out;
	}

	Matrix operator+(Matrix const& rhs) {
		Matrix out{};
		matrix_add<<<1, dim3(x, y, 1)>>>(this->inner, rhs.inner, out.inner);
		return out;
	}

	operator std::array<std::array<T, x>, y>() const {
		std::array<std::array<T, x>, y> ret{};
		gpu::copy_to_host(std::span(ret[0].data(), x * y), inner);
		return ret;
	}


	std::array<std::array<T, x>, y> to_host() const {
		return *this;
	}
};

enum class Axis { x, y, z };
template <Axis A, typename R>
	requires std::is_trivially_copyable_v<R>
__global__ void rotate_kernel(gpu::device_span3<R> span, int by) {
	const size_t i = blockIdx.x * blockDim.x + threadIdx.x;
	const size_t j = blockIdx.y * blockDim.y + threadIdx.y;

	const size_t nx = span.width();
	const size_t ny = span.height();
	const size_t nz = span.depth();

	size_t len;
	size_t x, y, z;

	if constexpr (A == Axis::x) {
		y = i; z = j;
		if (y >= ny || z >= nz) return;
		len = nx;
	} else if constexpr (A == Axis::y) {
		x = i; z = j;
		if (x >= nx || z >= nz) return;
		len = ny;
	} else if constexpr (A == Axis::z) {
		x = i; y = j;
		if (x >= nx || y >= ny) return;
		len = nz;
	} else {
		static_assert(false);
	}

	if (len <= 1)
		return;

	by %= static_cast<int>(len);
	if (by < 0)
		by += len;
	if (by == 0)
		return;

	auto at = [&](size_t k) -> R& {
		if constexpr (A == Axis::x) { return span[k, y, z]; }
		else if constexpr (A == Axis::y) { return span[x, k, z]; }
		else if constexpr (A == Axis::z) { return span[x, y, k]; }
		else { static_assert(false); }
	};

	auto reverse = [&](size_t a, size_t b) {
		while (a < b) {
			R tmp = at(a);
			at(a) = at(b);
			at(b) = tmp;
			++a;
			--b;
		}
	};

	reverse(0, len - 1);
	reverse(0, by - 1);
	reverse(by, len - 1);
}

TEMPLATE_COPYABLE(R)
void rotate(gpu::device_span3<R> span, int x, int y, int z)
	pre(
		(x != 0 && y == 0 && z == 0)
		|| (x == 0 && y != 0 && z == 0)
		|| (x == 0 && y == 0 && z != 0)
	)
{
	constexpr int BX = 16;
	constexpr int BY = 16;

	if (x != 0) {
		dim3 block(BX, BY);
		dim3 grid(
			(span.height + BX - 1) / BX,
			(span.depth  + BY - 1) / BY
		);
		rotate_kernel<Axis::x><<<grid, block>>>(span, x);
	}

	if (y != 0) {
		dim3 block(BX, BY);
		dim3 grid(
			(span.width + BX - 1) / BX,
			(span.depth + BY - 1) / BY
		);
		rotate_kernel<Axis::y><<<grid, block>>>(span, y);
	}

	if (z != 0) {
		dim3 block(BX, BY);
		dim3 grid(
			(span.width  + BX - 1) / BX,
			(span.height + BY - 1) / BY
		);
		rotate_kernel<Axis::z><<<grid, block>>>(span, z);
	}
}

TEMPLATE_COPYABLE(R)
auto solve_gs(
	Matrix<WIDTH, HEIGHT, R> phi3d,
	Matrix<WIDTH, HEIGHT, R> aw3d,
	Matrix<WIDTH, HEIGHT, R> ae3d,
	Matrix<WIDTH, HEIGHT, R> as3d,
	Matrix<WIDTH, HEIGHT, R> al3d,
	Matrix<WIDTH, HEIGHT, R> ah3d,
	Matrix<WIDTH, HEIGHT, R> su3d,
	Matrix<WIDTH, HEIGHT, R> ap3d,
	R tol_conv,
	size_t nmax
) {
	fmt::println!("solve_3d gs called, nmax: {}", nmax);

	for (size_t n : v::iota(0, n_max)) {
		phi3d = ((ae3d * rotate(phi3d, -1, 0, 0) + aw3d*rotate(phi3d, 1, 0, 0)
			+ an3d * rotate(phi3d, 0, -1, 0) + as3d*rotate(phi3d, 0, 1, 0)
			+ ah3d * rotate(phi3d, 0, 0, -1) + al3d*rotate(phi3d, 0, 0, 1))
			* axrank_conv + su3d) / ap3d;
	}

	auto res = ap3d * phi3d - ((ae3d))

	// for n in range(0,nmax):
	//    phi3d=((ae3d*np.roll(phi3d,-1,axis=0)+aw3d*np.roll(phi3d,1,axis=0) \
	//           +an3d*np.roll(phi3d,-1,axis=1)+as3d*np.roll(phi3d,1,axis=1) \
	//           +ah3d*np.roll(phi3d,-1,axis=2)+al3d*np.roll(phi3d,1,axis=2))*acrank_conv+su3d)/ap3d

	// res= ap3d*phi3d-\
	//   ((ae3d*np.roll(phi3d,-1,axis=0)+aw3d*np.roll(phi3d,1,axis=0) \
	//    +an3d*np.roll(phi3d,-1,axis=1)+as3d*np.roll(phi3d,1,axis=1) \
	//    +ah3d*np.roll(phi3d,-1,axis=2)+al3d*np.roll(phi3d,1,axis=2))*acrank_conv+su3d)

	// resid=np.sum(np.abs(res.flatten()))
	// return resid,phi3d
}

TEMPLATE_COPYABLE(R)
auto poisson_3d(usize size, int iterations) -> std::vector<R>
	pre(size > 0)
	post(r : r.size() == size * size * size)
{
	std::vector<R> ret{};
	ret.resize(size * size * size);

	Matrix<3, 3, f32> left {
		1.f, 0.f, 0.f,
		0.f, 2.f, 0.f,
		0.f, 0.f, 3.f
	};
	Matrix<3, 3, f32> right{
		1.f, 2.f, 3.f,
		4.f, 5.f, 6.f,
		7.f, 8.f, 9.f
	};

	Matrix<3, 3, f32> res = left * right;

	fmt::println("{}", res.to_host());

	return ret;
}
