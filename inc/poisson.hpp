#pragma once

#include <vector>

using num = float;

auto poisson(
	size_t nx,
	size_t ny,
	size_t nt,
	num x_min,
	num x_max,
	num y_min,
	num y_max
) -> std::vector<num>;
