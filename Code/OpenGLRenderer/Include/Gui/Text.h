#ifndef DEFAULT_FONT_PATH
#define FONT_PATH ""
#else
#define FONT_PATH DEFAULT_FONT_PATH
#endif

#include <glm/vec4.hpp>
#include <string>

namespace Text
{
	void Init ();
	bool LoadFont (const char *fontPath, unsigned *id);
	std::size_t UnloadFont (unsigned id);
	void Draw (unsigned font, std::string text,
				float x, float y, float scale,
				glm::vec4 color);
}