#include <Primitives/Quad.h>

#include <vector>
#include <Buffer.h>
#include <VertexArray.h>

Quad::Quad(float size, glm::vec3 pos)
{
	const std::vector<float> vertices = {
		pos.x - size / 2, pos.y - size / 2, 0.0f, 0.0f, 0.0f,
		pos.x - size / 2, pos.y + size / 2, 0.0f, 0.0f, 1.0f,
		pos.x + size / 2, pos.y - size / 2, 0.0f, 1.0f, 0.0f,
		pos.x + size / 2, pos.y + size / 2, 0.0f, 1.0f, 1.0f
	};

	const std::vector<unsigned int> indices = {
		0, 1, 2, 3
	};

	m_vao.Swap(VertexArray<float, unsigned>(
		VBufPtr<float>(new VertexBuffer<float>(GL_STATIC_DRAW, vertices)),
		IBufPtr<unsigned>(new IndexBuffer<unsigned>(GL_STATIC_DRAW, indices))));
	m_vao.setLayout({
		{ 3, GL_FLOAT },
		{ 2, GL_FLOAT }
	});
}

void
Quad::Draw() const
{
	m_vao.Bind();
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr);
	m_vao.UnBind();
}
