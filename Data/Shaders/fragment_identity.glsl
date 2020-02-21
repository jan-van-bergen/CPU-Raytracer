#version 420

layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec3 out_colour;

uniform sampler2D screen;

void main() {
	out_colour = texture2D(screen, in_uv).rgb;
}
