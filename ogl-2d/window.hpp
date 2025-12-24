#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

class Window {
	SDL_Window *window;
	SDL_GLContext gl_ctx;

      public:
	Window();
	~Window();

	bool run_event_loop();
};
