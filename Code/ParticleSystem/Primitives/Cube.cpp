#include <Primitives/Cube.h>

#include <vector>
#include <Buffer.h>
#include <VertexArray.h>

Cube::Cube(float size, glm::vec3 pos)
{
	const std::vector<float> vertices = {
		pos.x - size / 2, pos.y - size / 2, pos.z - size / 2,
		pos.x - size / 2, pos.y - size / 2, pos.z + size / 2,
		pos.x - size / 2, pos.y + size / 2, pos.z - size / 2,
		pos.x - size / 2, pos.y + size / 2, pos.z + size / 2,
		pos.x + size / 2, pos.y - size / 2, pos.z - size / 2,
		pos.x + size / 2, pos.y - size / 2, pos.z + size / 2,
		pos.x + size / 2, pos.y + size / 2, pos.z - size / 2,
		pos.x + size / 2, pos.y + size / 2, pos.z + size / 2,
	};
	
	const std::vector<unsigned int> indices = {
		4, 6, 5, 7, 3, 6, 2, 4, 0, 5, 1, 3, 0, 2
	};

	m_vao.Swap(VertexArray<float, unsigned>(
			VBufPtr<float>(new VertexBuffer<float>(GL_STATIC_DRAW, vertices)),
			IBufPtr<unsigned>(new IndexBuffer<unsigned>(GL_STATIC_DRAW, indices))));
	m_vao.setLayout({
		{ 3, GL_FLOAT, GL_FALSE }
	});
}

void
Cube::Draw() const
{
	m_vao.Bind();
	glDrawElements(GL_TRIANGLE_STRIP, 14, GL_UNSIGNED_INT, nullptr);
	m_vao.UnBind();
}
