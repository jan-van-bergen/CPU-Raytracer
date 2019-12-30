#pragma once
#include "Math.h"

struct CatmullRomSpline {
	struct KeyFrame {
		float   time;
		Vector3 value;
	};
	
	int              keyframe_count;
	const KeyFrame * keyframes;

	float time = 0.0f;
	int current_key_frame = 0;

	// Based on: https://www.iquilezles.org/www/articles/minispline/minispline.htm
	inline Vector3 get_point(float delta) {
		const static float coefficients[16] = {
			-1.0f,  2.0f, -1.0f, 0.0f,
			 3.0f, -5.0f,  0.0f, 2.0f,
			-3.0f,  4.0f,  1.0f, 0.0f,
			 1.0f, -1.0f,  0.0f, 0.0f 
		};

		time += delta;

		if (time >= keyframes[keyframe_count - 1].time) {
			time = 0;
			current_key_frame = 0;
		}

		// Check if we should advance one or more keyframes
		while (keyframes[current_key_frame].time < time) current_key_frame++;

		// Find interpolation value
		float x = (time - keyframes[current_key_frame - 1].time) / (keyframes[current_key_frame].time - keyframes[current_key_frame - 1].time);

		Vector3 result;

		// Add basis functions
		for (int i = 0; i < 4; i++) {
			const float * coeff = coefficients + 4 * i;
			
			int keyframe_index = Math::clamp(current_key_frame + i - 2, 0, keyframe_count - 1); 

			// Evaluate polynomial
			result += 0.5f * (((coeff[0] * x + coeff[1]) * x + coeff[2]) * x + coeff[3]) * keyframes[keyframe_index].value;
		}

		return result;
	}
};
