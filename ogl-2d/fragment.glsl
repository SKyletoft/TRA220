#version 330 core

in vec3 pos;

out vec4 out_colour;

uniform vec3 triangle_colour;
uniform int width;
uniform int height;

void iteration(int x, int y) {

}

void main() {
	vec3 pos_ = (pos + 1) / 2;
	int x = int(pos_.x * 4) * 256 / 4;
	int y = int(pos_.y * 4) * 256 / 4;
	out_colour = vec4(
		float(x) / 256,
		float(y) / 256,
		pos_.x,
		1.0
	);
}
