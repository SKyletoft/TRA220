#include "compute.hpp"

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include <array>
#include <fstream>
#include <print>
#include <streambuf>
#include <string>

static constexpr std::array<float, 6 * 3> verts {
	-1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,

	-1.0f,  1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,
};

std::string get_shader_info_log(GLuint obj) {
	int log_length = 0;
	int chars_written = 0;
	char* tmp_log;
	std::string log;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &log_length);

	if(log_length > 0)
	{
		tmp_log = new char[log_length];
		glGetShaderInfoLog(obj, log_length, &chars_written, tmp_log);
		log = tmp_log;
		delete[] tmp_log;
	}

	return log;
}

ComputeContext::ComputeContext() {
	GLuint position_buffer;
	glGenBuffers(1, &position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &vertex_array_object);
	glBindVertexArray(vertex_array_object);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

	glEnableVertexAttribArray(0);
	GLuint vertex_shader   = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string vs_src     = {
		#embed "vertex.glsl"
	};
	std::string fs_src = {
		#embed "fragment.glsl"
	};

	std::println("{}", __LINE__);

	const char *vs = vs_src.c_str();
	const char *fs = fs_src.c_str();

	glShaderSource(vertex_shader, 1, &vs, NULL);
	glShaderSource(fragment_shader, 1, &fs, NULL);

	glCompileShader(vertex_shader);
	int compileOK;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compileOK);
	if (!compileOK) {
		std::println("{}", get_shader_info_log(vertex_shader));
		throw std::runtime_error("Failed to compile vertex shader");
	}

	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compileOK);
	if (!compileOK) {
		std::println("{}", get_shader_info_log(fragment_shader));
		throw std::runtime_error("Failed to compile fragment shader");
	}

	shader_program = glCreateProgram();
	glAttachShader(shader_program, fragment_shader);
	glAttachShader(shader_program, vertex_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glLinkProgram(shader_program);
	GLint linkOk = 0;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &linkOk);
	if (!linkOk) {
		throw std::runtime_error("Failed to link shaders");
	}
}

void ComputeContext::compute() {
	glViewport(0, 0, 500, 500);

	glClearColor(
		0.0,
		0.0,
		0.0,
		1.0
	);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glUseProgram(this->shader_program);
	glUniform3fv(
		glGetUniformLocation(this->shader_program, "triangle_colour"),
		1,
		std::array<float, 3>{1.0f, 1.0f, 1.0f}.data()
	);

	glBindVertexArray(this->vertex_array_object);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glUseProgram(0);
}
