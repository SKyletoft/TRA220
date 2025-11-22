#pragma once

#include <vector>

auto poisson(
	size_t nx,
	size_t ny,
	size_t nt,
	float x_min,
	float x_max,
	float y_min,
	float y_max
) -> std::vector<float>;
