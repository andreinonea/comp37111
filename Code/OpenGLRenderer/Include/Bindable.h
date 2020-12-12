#ifndef __IBINDABLE_H__
#define __IBINDABLE_H__

struct IBindable
{
	virtual void Bind() const = 0;
	virtual void UnBind() const = 0;
};

#endif // __IBINDABLE_H__
