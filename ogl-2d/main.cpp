#include "compute.hpp"
#include "window.hpp"

#include <stdexcept>
#include <print>

int main(int argc, char *argv[]) {
	Window w{};
	ComputeContext ctx{};

	while (w.run_event_loop()) {
		try {
			ctx.compute();
		} catch (std::runtime_error const &e) {
			std::println("{}", e.what());
		}
	}

	return 0;
}
