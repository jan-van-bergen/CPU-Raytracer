#include "Skybox.h"

#include <algorithm>

#include "Util.h"

Skybox::Skybox() {
	left  = Texture::load(DATA_PATH("Skybox/Left.png"));
	right = Texture::load(DATA_PATH("Skybox/Right.png"));
	up    = Texture::load(DATA_PATH("Skybox/Up.png"));
	down  = Texture::load(DATA_PATH("Skybox/Down.png"));
	front = Texture::load(DATA_PATH("Skybox/Front.png"));
	back  = Texture::load(DATA_PATH("Skybox/Back.png"));
}

#define UV(x) ((x + 1.0f) * 0.5f)

Vector3 Skybox::sample(const Vector3 & direction) const {
    float min = std::min(direction.x, std::min(direction.y, direction.z));
	float max = std::max(direction.x, std::max(direction.y, direction.z));
	
    if (-min > max) {
		float inv_min = 1.0f / min;

        if (min == direction.x) {
            return right->sample(
				UV(direction.z * inv_min), 
				UV(direction.y * inv_min)
			);
		} else if (min == direction.y) {
            return down->sample(
				UV(-direction.x * inv_min), 
				UV(-direction.z * inv_min)
			);
		} else {
            return front->sample(
				UV(-direction.x * inv_min), 
				UV( direction.y * inv_min)
			);
		}
    } else {
		float inv_max = 1.0f / max;

        if (max == direction.x) {
            return left->sample(
				UV( direction.z * inv_max), 
				UV(-direction.y * inv_max)
			);
		} else if (max == direction.y) {
            return up->sample(
				UV( direction.x * inv_max), 
				UV(-direction.z * inv_max)
			);
		} else {
            return back->sample(
				UV(-direction.x * inv_max), 
				UV(-direction.y * inv_max)
			);
		}
    }
}
