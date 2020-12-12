#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifdef _WIN32
#define ASSERT(x) if (!(x)) __debugbreak()
#define M_PI glm::pi<float>()
#else
#include <cassert>
#define ASSERT(x) assert(x);
#endif

#endif // !__PLATFORM_H__
