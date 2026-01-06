#pragma once

using GLuint = unsigned int;

class ComputeContext {
	GLuint vertex_array_object;
	GLuint shader_program;

public:
	ComputeContext();
	void compute();
};
