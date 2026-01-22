#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <fstream>
#include <print>
#include <sstream>
#include <string>
#include <vector>

using i32   = int32_t;
using u32   = uint32_t;
using usize = size_t;
using num   = float;

class PoissonSolver {
      private:
	SDL_Window *window;
	SDL_GLContext gl_context;

	GLuint fbo;
	GLuint p_tex[2]; // ping-pong textures for p and pd
	GLuint b_tex;
	GLuint quad_vao, quad_vbo;
	GLuint compute_program;
	GLuint display_program;

	usize nx, ny;
	num dx2, dy2;
	int current_tex;

	const char *quad_vert = R"(
	#version 330 core
	layout(location = 0) in vec2 pos;
	out vec2 tex_coord;
	void main() {
	    gl_Position = vec4(pos, 0.0, 1.0);
	    tex_coord = pos * 0.5 + 0.5;
	}
    )";

	const char *compute_frag = R"(
	#version 330 core
	uniform sampler2D pd_tex;
	uniform sampler2D b_tex;
	uniform float dx2;
	uniform float dy2;
	uniform vec2 resolution;
	out vec4 frag_color;
	in vec2 tex_coord;

	void main() {
	    vec2 inv_res = 1.0 / resolution;
	    float i = tex_coord.y * resolution.y;
	    float j = tex_coord.x * resolution.x;

	    // Boundary conditions
	    if (i < 1.0 || j < 1.0 || i >= resolution.y - 1.0 || j >= resolution.x - 1.0) {
		frag_color = vec4(0.0);
		return;
	    }

	    // Sample neighbors
	    float pd_center = texture(pd_tex, tex_coord).r;
	    float pd_right = texture(pd_tex, tex_coord + vec2(inv_res.x, 0.0)).r;
	    float pd_left = texture(pd_tex, tex_coord - vec2(inv_res.x, 0.0)).r;
	    float pd_up = texture(pd_tex, tex_coord + vec2(0.0, inv_res.y)).r;
	    float pd_down = texture(pd_tex, tex_coord - vec2(0.0, inv_res.y)).r;
	    float b_val = texture(b_tex, tex_coord).r;

	    // Poisson iteration
	    float p_new = ((pd_right + pd_left) * dy2 + (pd_up + pd_down) * dx2 - b_val * dx2 * dy2)
			  / (2.0 * (dx2 + dy2));

	    frag_color = vec4(p_new, 0.0, 0.0, 1.0);
	}
    )";

	const char *display_frag = R"(
	#version 330 core
	uniform sampler2D p_tex;
	in vec2 tex_coord;
	out vec4 frag_color;

	void main() {
	    float val = texture(p_tex, tex_coord).r;
	    // Map value to color with higher sensitivity
	    // Normalize to roughly -50 to +50 range
	    float normalized = val / 50.0;
	    vec3 color;
	    if (normalized > 0.0) {
		color = vec3(min(normalized, 1.0), 0.0, 0.0);  // Red for positive
	    } else {
		color = vec3(0.0, 0.0, min(-normalized, 1.0)); // Blue for negative
	    }
	    // Add green channel to show magnitude
	    color.g = abs(normalized) * 0.3;
	    frag_color = vec4(color, 1.0);
	}
    )";

	GLuint compile_shader(GLenum type, const char *source) {
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char log[512];
			glGetShaderInfoLog(shader, 512, nullptr, log);
			std::print("Shader compilation error: {}\n", log);
		}
		return shader;
	}

	GLuint create_program(const char *vert_src, const char *frag_src) {
		GLuint vert    = compile_shader(GL_VERTEX_SHADER, vert_src);
		GLuint frag    = compile_shader(GL_FRAGMENT_SHADER, frag_src);
		GLuint program = glCreateProgram();
		glAttachShader(program, vert);
		glAttachShader(program, frag);
		glLinkProgram(program);

		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			char log[512];
			glGetProgramInfoLog(program, 512, nullptr, log);
			std::print("Program linking error: {}\n", log);
		}

		glDeleteShader(vert);
		glDeleteShader(frag);
		return program;
	}

	void setup_quad() {
		float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

		glGenVertexArrays(1, &quad_vao);
		glGenBuffers(1, &quad_vbo);
		glBindVertexArray(quad_vao);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	}

	GLuint create_texture(usize width, usize height) {
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT,
			     nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		return tex;
	}

      public:
	PoissonSolver(usize width, usize height, num x_min, num x_max, num y_min, num y_max)
	    : nx(width), ny(height), current_tex(0) {

		SDL_Init(SDL_INIT_VIDEO);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		window = SDL_CreateWindow("Poisson Solver", SDL_WINDOWPOS_CENTERED,
					  SDL_WINDOWPOS_CENTERED, 800, 800,
					  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

		gl_context = SDL_GL_CreateContext(window);
		glewInit();

		num dx = (x_max - x_min) / num(nx - 1);
		num dy = (y_max - y_min) / num(ny - 1);
		dx2    = dx * dx;
		dy2    = dy * dy;

		// Create textures
		p_tex[0] = create_texture(nx, ny);
		p_tex[1] = create_texture(nx, ny);
		b_tex    = create_texture(nx, ny);

		// Initialize b texture with source terms
		std::vector<num> b_data(nx * ny, 0.0f);
		b_data[ny / 4 * nx + nx / 4]         = 100.0f;
		b_data[3 * ny / 4 * nx + 3 * nx / 4] = -100.0f;
		glBindTexture(GL_TEXTURE_2D, b_tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RED, GL_FLOAT, b_data.data());

		// Also initialize p textures to ensure they're not undefined
		std::vector<num> zero_data(nx * ny, 0.0f);
		glBindTexture(GL_TEXTURE_2D, p_tex[0]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RED, GL_FLOAT,
				zero_data.data());
		glBindTexture(GL_TEXTURE_2D, p_tex[1]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RED, GL_FLOAT,
				zero_data.data());

		// Create FBO
		glGenFramebuffers(1, &fbo);

		// Create shader programs
		compute_program = create_program(quad_vert, compute_frag);
		display_program = create_program(quad_vert, display_frag);

		setup_quad();
	}

	~PoissonSolver() {
		glDeleteTextures(2, p_tex);
		glDeleteTextures(1, &b_tex);
		glDeleteFramebuffers(1, &fbo);
		glDeleteVertexArrays(1, &quad_vao);
		glDeleteBuffers(1, &quad_vbo);
		glDeleteProgram(compute_program);
		glDeleteProgram(display_program);
		SDL_GL_DeleteContext(gl_context);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void iterate() {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				       p_tex[1 - current_tex], 0);
		glViewport(0, 0, nx, ny);

		glUseProgram(compute_program);
		glUniform1i(glGetUniformLocation(compute_program, "pd_tex"), 0);
		glUniform1i(glGetUniformLocation(compute_program, "b_tex"), 1);
		glUniform1f(glGetUniformLocation(compute_program, "dx2"), dx2);
		glUniform1f(glGetUniformLocation(compute_program, "dy2"), dy2);
		glUniform2f(glGetUniformLocation(compute_program, "resolution"), nx, ny);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, p_tex[current_tex]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, b_tex);

		glBindVertexArray(quad_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		current_tex = 1 - current_tex;
	}

	void display() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		glViewport(0, 0, w, h);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(display_program);
		glUniform1i(glGetUniformLocation(display_program, "p_tex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, p_tex[current_tex]);

		glBindVertexArray(quad_vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		SDL_GL_SwapWindow(window);
	}

	void print_grid_12x12() {
		std::vector<num> data(nx * ny);
		glBindTexture(GL_TEXTURE_2D, p_tex[current_tex]);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data.data());

		std::print("\n12x12 Grid Sample:\n");
		for (int i = 0; i < 12; ++i) {
			for (int j = 0; j < 12; ++j) {
				int sample_y = (i * ny) / 12;
				int sample_x = (j * nx) / 12;
				num val      = data[sample_y * nx + sample_x];
				std::print("{:8.2f} ", val);
			}
			std::print("\n");
		}
		std::print("\n");
	}

	void print_full_grid() {
		std::vector<num> data(nx * ny);
		glBindTexture(GL_TEXTURE_2D, p_tex[current_tex]);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data.data());

		std::print("{}\t{}\t0\n", ny, nx);
		for (size_t j = 0; j < ny; ++j) {
			for (size_t i = 0; i < nx; ++i) {
				num z = data[j * nx + i];
				std::print("{}\t{}\t{}\n", i, j, z);
			}
		}
	}

	void solve(usize iterations) {
		for (usize i = 0; i < iterations; ++i) {
			iterate();
			if (i % 10 == 0) {
				display();
			}
		}
	}

	void run(usize iterations_per_frame = 10) {
		bool running = true;
		SDL_Event event;

		while (running) {
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT) {
					running = false;
				}
				if (event.type == SDL_KEYDOWN
				    && event.key.keysym.sym == SDLK_ESCAPE) {
					running = false;
				}
			}

			for (usize i = 0; i < iterations_per_frame; ++i) {
				iterate();
			}
			display();
		}
	}
};

int main() {
	PoissonSolver solver(5000, 5000, 0.0f, 2.0f, 0.0f, 2.0f);

	// Run some iterations before starting the interactive loop
	// std::print(stderr, "Running initial iterations...\n");
	for (int i = 0; i < 10000; ++i) {
		solver.iterate();
		if (i % 100 == 0) {
			// std::print(stderr, "Iteration {}\n", i);
			// solver.print_grid_12x12();
		}
	}

	// std::print(stderr, "Final result:\n");
	// solver.print_grid_12x12();

	// Print full grid to stdout for piping to graphing tool
	// solver.print_full_grid();

	// Uncomment to see interactive visualization:
	// solver.run(10);

	return 0;
}
