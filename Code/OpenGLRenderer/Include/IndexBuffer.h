#ifndef __INDEXBUFFER_H__
#define __INDEXBUFFER_H__

#include <GL/glew.h>
#include <cstdint>

class IndexBuffer
{
private:
	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer operator=(const IndexBuffer&) = delete;

public:
	IndexBuffer(const unsigned int *data, const std::size_t count)
		: m_Count(count)
	{
		//ASSERT(sizeof(unsigned int) == sizeof(GLuint))

		glGenBuffers(1, &m_InternalId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_InternalId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
	}

	~IndexBuffer()
	{
		glDeleteBuffers(1, &m_InternalId);
	}

	inline void Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_InternalId);
	}
	inline void Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	inline unsigned int GetInternalId() { return m_InternalId; }
	inline std::size_t GetCount() { return m_Count; }

private:
	unsigned int m_InternalId;
	std::size_t m_Count;
};

#endif // __INDEXBUFFER_H_
