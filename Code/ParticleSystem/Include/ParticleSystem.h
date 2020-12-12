#ifndef __PARTICLESYSTEM_H__
#define __PARTICLESYSTEM_H__

#include <glm/vec3.hpp>

struct Particle
{
	float lifetime = 1.0f;
	bool active = false;

	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale = { 0.5f, 0.5f, 0.5f };

	float mass = 1.0f;
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 acceleration = { 0.0f, 0.0f, 0.0f };
};


#endif // !__PARTICLESYSTEM_H__