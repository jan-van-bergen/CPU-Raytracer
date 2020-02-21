#version 420

layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec3 out_colour;

uniform sampler2D screen;

uniform vec2 inv_screen_size;

const float FXAA_REDUCE_MIN = 1.0f / 128.0f;
const float FXAA_REDUCE_MUL = 1.0f / 8.0f;
const float FXAA_SPAN_MAX   = 8.0f;

// Get screen, gamma corrected
vec3 get_screen(vec2 uv) {
	return pow(texture2D(screen, uv).rgb, vec3(1.0f / 2.2f));
}

// Based on: https://github.com/BennyQBD/3DEngineCpp
vec3 fxaa() {
	const vec3 luma = vec3(0.299f, 0.587f, 0.114f);

	// Sample the screen in an X-shaped pattern around the current pixel and convert to luminosity
	float luma_tl = dot(luma, get_screen(in_uv + vec2(-inv_screen_size.x, -inv_screen_size))); 
	float luma_tr = dot(luma, get_screen(in_uv + vec2( inv_screen_size.x, -inv_screen_size))); 
	float luma_bl = dot(luma, get_screen(in_uv + vec2(-inv_screen_size.x,  inv_screen_size))); 
	float luma_br = dot(luma, get_screen(in_uv + vec2( inv_screen_size.x,  inv_screen_size))); 
	float luma_m  = dot(luma, get_screen(in_uv).rgb);

	// Calculate the min and the max out of all the luminosity values
	float luma_min = min(min(min(luma_tl, luma_tr), min(luma_bl, luma_br)), luma_m);
	float luma_max = max(max(max(luma_tl, luma_tr), max(luma_bl, luma_br)), luma_m);
	
	// Obtain gradient
	vec2 blur_direction = vec2(
		(luma_bl + luma_br) - (luma_tl + luma_tr),
		(luma_tl + luma_bl) - (luma_tr + luma_br)
	);
	
	// Make sure the blur direction is within reasonable range
	float reduce = max(FXAA_REDUCE_MIN, (luma_tl + luma_tr + luma_bl + luma_br) * 0.25f * FXAA_REDUCE_MUL);
	float adjust = 1.0f / (min(
		abs(blur_direction.x), 
		abs(blur_direction.y)
	) + reduce);
	
	blur_direction = clamp(blur_direction * adjust, vec2(-FXAA_SPAN_MAX), vec2(FXAA_SPAN_MAX)) * inv_screen_size;
	
	vec3 result_a = 0.5f * (
		get_screen(in_uv + blur_direction * (1.0f / 3.0f - 0.5f)).rgb +
		get_screen(in_uv + blur_direction * (2.0f / 3.0f - 0.5f)).rgb
	);
	vec3 result_b = 0.5f * (
		get_screen(in_uv + blur_direction * (0.0f / 3.0f - 0.5f)).rgb +
		get_screen(in_uv + blur_direction * (3.0f / 3.0f - 0.5f)).rgb
	);
	
	// Average results
	vec3 result = 0.5f * (result_a + result_b);
	
	float luma_result = dot(luma, result);
	
	// Check if the luminosity is outside of the range we expect
	if (luma_result < luma_min || luma_result > luma_max) {
		return result_a; // Return only the average of the first two samples
	} else {
		return result; // Return the average of all 4 samples
	}
}

void main() {
	out_colour = fxaa();
}
