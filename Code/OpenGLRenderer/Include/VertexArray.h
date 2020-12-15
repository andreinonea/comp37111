#ifndef __VERTEXARRAY_H__
#define __VERTEXARRAY_H__

#include <GL/glew.h>

#include <Bindable.h>
#include <Buffer.h>
#include <memory>

#include <Platform.h>

template <typename VT>
using VBufPtr = std::unique_ptr<VertexBuffer<VT>>;
template <typename IT>
using IBufPtr = std::unique_ptr<IndexBuffer<IT>>;

struct VertexAttribute
{
	unsigned count;
	unsigned type;
	bool normalized;

	VertexAttribute(unsigned count, unsigned type, bool normalized = false)
		: count{count}
		, type{type}
		, normalized{normalized}
	{}

	std::size_t size() const
	{
		switch (type)
		{
			case GL_FLOAT:
				return count * sizeof(float);
			default:
				throw std::runtime_error("Invalid type");
		}
	}
};

template <typename VType, typename IType>
class VertexArray : public IBindable
{
public:
	unsigned int m_id;
	VBufPtr<VType> m_vbo;
	IBufPtr<IType> m_ibo;

public:
	void Swap(VertexArray&& other)
	{
		std::swap(m_id, other.m_id);
		std::swap(m_vbo, other.m_vbo);
		std::swap(m_ibo, other.m_ibo);
	}

public:
	explicit VertexArray()
	{
		glGenVertexArrays(1, &m_id);
		if (! m_id)
			throw std::runtime_error ("Unable to generate vertex array");
	}

	VertexArray(VBufPtr<VType> pvbo, IBufPtr<IType> pibo,
			std::initializer_list<VertexAttribute> attribs = {})
		: VertexArray()
	{
		m_vbo.swap(pvbo);
		m_ibo.swap(pibo);

		setLayout(attribs);
	}

	~VertexArray()
	{
		glDeleteVertexArrays(1, &m_id);
	}

public:
	void setLayout(std::initializer_list<VertexAttribute> attribs)
	{
		unsigned stride = 0;
		intptr_t offset = 0;
		unsigned idx = 0;

		for (const VertexAttribute& a : attribs)
		{
			stride += static_cast<unsigned>(a.size());
		}

		glBindVertexArray(m_id);
		m_vbo->Bind();

		for (const VertexAttribute& a : attribs)
		{
			glEnableVertexAttribArray(idx);
			glVertexAttribPointer(idx, a.count, a.type, a.normalized,
					stride, reinterpret_cast<void*>(offset));

			offset += static_cast<unsigned>(a.size());
			idx++;
		}

		m_vbo->UnBind();
		glBindVertexArray(0);

		ASSERT(offset == stride);
	}

public:
	inline void Bind() const override
	{
		glBindVertexArray(m_id);
		m_vbo->Bind();
		m_ibo->Bind();
	}
	
	inline void UnBind() const override
	{
		glBindVertexArray(0);
		m_vbo->UnBind();
		m_ibo->UnBind();
	}
};

#endif // __VERTEXBUFFER_H_
