#ifndef __VERTEXBUFFER_H__
#define __VERTEXBUFFER_H__

#include <GL/glew.h>
#include <cstdint>

class VertexBuffer
{
private:
	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer operator=(const VertexBuffer&) = delete;

public:
	template <typename T>
	VertexBuffer(const T *data, const std::size_t count)
	{
		glGenBuffers(1, &m_InternalId);
		glBindBuffer(GL_ARRAY_BUFFER, m_InternalId);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(T), data, GL_STATIC_DRAW);
	}

	~VertexBuffer()
	{
		glDeleteBuffers(1, &m_InternalId);
	}

	inline void Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_InternalId);
	}

	inline void Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	inline unsigned int getInternalId() { return m_InternalId; }

private:
	unsigned int m_InternalId;
};

#endif // __VERTEXBUFFER_H_
