#pragma once
#include "Texture.h"

#include "Vector3.h"

struct Skybox {
private:
	const Texture * left;	
	const Texture * right;	
	const Texture * up;	
	const Texture * down;	
	const Texture * front;	
	const Texture * back;	

public:
	Skybox();

	Vector3 sample(const Vector3 & direction) const;
};
