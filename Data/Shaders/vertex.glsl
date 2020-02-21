#version 420

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec2 out_uv;

void main() {
	out_uv = in_uv;
	
	gl_Position = vec4(in_position, 0.0f, 1.0f);
}
