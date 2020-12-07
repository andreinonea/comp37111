#ifndef __RENDERER_OPENGL_H__
#define __RENDERER_OPENGL_H__

#include <GL/glew.h>
#include <fmt/core.h>

#ifdef WIN32
	#define ASSERT(x) if (!(x)) __debugbreak()
#else
	#define ASSERT(x) assert(x);
#endif

void APIENTRY glDebugOutput(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char *msg, const void *data);


#endif // __RENDERER_OPENGL_H__
