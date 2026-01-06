#include "window.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <format>
#include <stdexcept>
#include <stdio.h>

Window::Window() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw std::runtime_error(std::format("SDL_Init failed: {}", SDL_GetError()));
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	this->window = SDL_CreateWindow(
		"SDL2 OpenGL 2.0",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		500,
		500,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
	);

	if (!window) {
		auto err = std::format("SDL_CreateWindow failed: {}", SDL_GetError());
		SDL_Quit();
		throw std::runtime_error(err);
	}

	this->gl_ctx = SDL_GL_CreateContext(this->window);
	if (!gl_ctx) {
		auto err = std::format("SDL_GL_CreateContext failed: {}", SDL_GetError());
		SDL_DestroyWindow(this->window);
		SDL_Quit();
		throw std::runtime_error(err);
	}

	glewInit();

	SDL_GL_SetSwapInterval(1);

	glViewport(0, 0, 800, 600);
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
}

Window::~Window() {
	SDL_GL_DeleteContext(this->gl_ctx);
	SDL_DestroyWindow(this->window);
	SDL_Quit();
}

bool Window::run_event_loop() {
	bool running = true;
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			running = false;
		}
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == 'q') {
			running = false;
		}
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
			running = false;
		}
	}

	SDL_GL_SwapWindow(window);
	return running;
}
