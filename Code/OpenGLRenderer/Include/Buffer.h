#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <Bindable.h>

#include <stdexcept>
#include <vector>
#include <GL/glew.h>

template <typename T, unsigned type>
class Buffer : public IBindable
{
private:
	unsigned int m_id;
	unsigned int m_hint;

public:
	void Swap(Buffer<T, type>&& other)
	{
		std::swap(m_id, other.m_id);
		std::swap(m_hint, other.m_hint);
	}

	Buffer(const Buffer& other)
	{
		std::size_t size = other.size();

		Buffer tmp(other.m_hint, size);
		glBindBuffer(GL_COPY_WRITE_BUFFER, tmp.m_id);
		glBindBuffer(GL_COPY_READ_BUFFER, other.m_id);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
				0, 0, size);

		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
		glBindBuffer(GL_COPY_READ_BUFFER, 0);

		Swap(std::move(tmp));
	}

	Buffer operator=(const Buffer& other)
	{
		Buffer tmp(other);
		Swap(other);

		return *this;
	}

	Buffer(Buffer&& other)
	{
		Swap(other);
	}

	Buffer operator=(Buffer&& other)
	{
		Swap(Buffer(other));
		return *this;
	}

public:
	~Buffer()
	{
		if (m_id)
			glDeleteBuffers(1, &m_id);
	}

	explicit Buffer() : m_id{0}, m_hint{0}
	{
		glCreateBuffers(1, &m_id);
		if (! m_id)
			throw std::runtime_error("Unable to create buffer");
	}

	explicit Buffer(unsigned hint, std::vector<T> data)
		: Buffer(hint, data.size(), data.data())
	{}

	explicit Buffer(unsigned hint, std::size_t size,
			const T *data = nullptr)
		: Buffer()
	{
		if (! size)
			throw std::runtime_error("Buffer length cannot be zero");

		setData(hint, size, data);
	}

	inline void setData(unsigned hint, std::vector<T> data)
	{
		setData(hint, data.size(), data.data());
	}

	inline void setData(unsigned hint, std::size_t count, const T *data)
	{
		Bind();
		glBufferData(type, count * sizeof(T), data, hint);
		UnBind();

		m_hint = hint;
	}

	inline void setSubData (unsigned offset, std::vector<T> data)
	{
		setSubData (offset, data.size (), data.data ());
	}

	inline void setSubData (unsigned offset, std::size_t count, const T *data)
	{
		// Bind ();
		glBufferSubData (type, offset, count * sizeof (T), data);
		// UnBind ();
	}

	inline std::size_t size() const
	{
		int res;

		Bind();
		glGetBufferParameteriv(type, GL_BUFFER_SIZE, &res);
		UnBind();

		return res;
	}

public:
	inline void Bind() const override
	{
		glBindBuffer(type, m_id);
	}

	void UnBind() const override
	{
		glBindBuffer(type, 0);
	}
};

template <typename T>
using VertexBuffer = Buffer<T, GL_ARRAY_BUFFER>;

template <typename T>
using IndexBuffer = Buffer<T, GL_ELEMENT_ARRAY_BUFFER>;

#endif // __BUFFER_H__
