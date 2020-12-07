#include <VertexArray.h>

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_InternalId);
	glBindVertexArray(m_InternalId);
}

void VertexArray::AddBuffer(VertexBuffer vbo)
{

}

void VertexArray::AddLayout(BufferLayout layout)
{

}

void VertexArray::Bind()
{
	glBindVertexArray(m_InternalId);
}

void VertexArray::Unbind()
{
	glBindVertexArray(0);
}
