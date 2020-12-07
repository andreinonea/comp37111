#ifndef __VERTEXARRAY_H__
#define __VERTEXARRAY_H__

#include <GL/glew.h>

#include "VertexBuffer.h"
#include "BufferLayout.h"

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void AddBuffer(VertexBuffer vbo);
	void AddLayout(BufferLayout layout);

	void Bind() const;
	void Unbind() const;

	inline unsigned int getInternalId() { return m_InternalId; }

private:
	unsigned int m_InternalId;
};

#endif // __VERTEXBUFFER_H_
