#version 330 core

in vec3 pos;

out vec4 out_colour;

uniform vec3 triangle_colour;

void main() {
	out_colour = vec4(pos, 1.0);
}
