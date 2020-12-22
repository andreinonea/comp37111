#ifndef __PARTICLESYSTEM_H__
#define __PARTICLESYSTEM_H__

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define MAX_PARTICLES 10000
constexpr glm::vec3 GRAVITY (0.0f, -9.8f, 0.0f);

struct IParticle
{
	virtual ~IParticle() = default;

	float lifespan = 1.0f;
	float lifetime = 0.0f;
	bool active = false;
	bool child = false;
	bool tracked = false;

	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

	// glm::vec3 rgb = { 1.0f, 1.0f, 1.0f };
	glm::vec4 rgba = { 1.0f, 1.0f, 1.0f, 1.0f };

	float mass = 1.0f;
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 acceleration = GRAVITY * mass;

	struct previous
	{
		float lifetime;
		float alpha;
		glm::vec3 position;
		glm::vec3 velocity;
	} previous;
};

struct FireworkParticle : public IParticle
{
	int recursionLevel = 3;
	unsigned explosionFactor = 31;
};

struct FireworkChildParticle : public FireworkParticle
{

};


#endif // !__PARTICLESYSTEM_H__