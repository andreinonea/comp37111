#ifndef __PRIMITIVES_CUBE_H__
#define __PRIMITIVES_CUBE_H__

#include <VertexArray.h>
#include <glm/vec3.hpp>

class Cube
{
public:
	Cube(float size = 1.0f, glm::vec3 position = {0, 0, 0});

	void Draw() const;

private:
	VertexArray<float, unsigned> m_vao;
	unsigned int m_program;
};

#endif // __PRIMITIVES_CUBE_H__
