#ifndef __PRIMITIVES_QUAD_H__
#define __PRIMITIVES_QUAD_H__

#include <VertexArray.h>
#include <glm/vec2.hpp>

class Quad
{
public:
	Quad(float size = 1.0f, glm::vec2 position = { 0.0f, 0.0f });

	void Draw() const;

private:
	VertexArray<float, unsigned> m_vao;
	unsigned int m_program;
};

#endif // __PRIMITIVES_CUBE_H__
