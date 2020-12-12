#ifndef __PARTICLESYSTEM_H__
#define __PARTICLESYSTEM_H__

#include <glm/vec3.hpp>

struct Particle
{
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	bool active = false;
};


#endif // !__PARTICLESYSTEM_H__