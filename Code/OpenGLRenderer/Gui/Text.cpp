#include <Gui/Text.h>

#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <iostream>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>

#include <Shader.h>
#include <VertexArray.h>

namespace Text
{
	namespace
	{
		struct Glyph
		{
			unsigned id; // ID handle of the glyph texture
			glm::ivec2 size;      // Size of glyph
			glm::ivec2 bearing;   // Offset from baseline to left/top of glyph
			unsigned advance;   // Horizontal offset to advance to next glyph
		};

		static std::map<unsigned, std::map<char, Glyph>> m_fontCache;
		static unsigned m_idAllocator = 1;

		static const std::vector<unsigned int> m_indices = {
			0, 1, 2, 3
		};

		static std::unique_ptr<VertexArray<float, unsigned>> m_vao;
	}

	void Init ()
	{
		m_vao.reset(new VertexArray<float, unsigned>(
			VBufPtr<float> (
				new VertexBuffer<float> (GL_DYNAMIC_DRAW, 16)),
			IBufPtr<unsigned> (
				new IndexBuffer<unsigned> (GL_STATIC_DRAW, m_indices)),
			{ { 4, GL_FLOAT } }));
	}

	bool LoadFont (const char *fontPath, unsigned *id)
	{
		FT_Library ft;
		if (FT_Init_FreeType (&ft))
		{
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library"
				<< '\n';
			return 1;
		}

		FT_Face face;
		if (FT_New_Face (ft, fontPath, 0, &face))
		{
			FT_Done_FreeType (ft);
			std::cout << "ERROR::FREETYPE: Failed to load font at path: "
				<< fontPath << '\n';
			return 1;
		}

		std::map<char, Glyph> font;
		FT_Set_Pixel_Sizes (face, 0, 48);
		glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

		int backup;
		glGetIntegerv (GL_TEXTURE_BINDING_2D, &backup);

		for (unsigned char c = 0; c < 128; c++)
		{
			if (FT_Load_Char (face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph for '"
					<< c << "'" << '\n';
				continue;
			}

			unsigned texId;
			glGenTextures (1, &texId);
			
			glBindTexture (GL_TEXTURE_2D, texId);
			glTexImage2D (
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

			glTexParameteri (GL_TEXTURE_2D,
								GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D,
								GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			font.emplace (std::pair<char, Glyph> (
				c, {
					texId,
					glm::ivec2 (
						face->glyph->bitmap.width,
						face->glyph->bitmap.rows),
					glm::ivec2 (
						face->glyph->bitmap_left,
						face->glyph->bitmap_top),
					static_cast<unsigned>(face->glyph->advance.x)
				}
				)
			);
		}

		*id = m_idAllocator++;
		m_fontCache.emplace (std::pair<unsigned, std::map<char, Glyph>> (
			*id, font)
		);

		glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
		glBindTexture (GL_TEXTURE_2D, backup);
		FT_Done_Face (face);
		FT_Done_FreeType (ft);

		return 0;
	}

	std::size_t UnloadFont (unsigned id)
	{
		if (m_fontCache.count (id))
			return m_fontCache.erase (id);
		else
		{
			std::cout << "Text::UnloadFont::Font with id'" << id
				<< "' is not loaded" << '\n';
			return 0;
		}
	}

	void Draw (unsigned font, std::string text,
				float x, float y, float scale,
				glm::vec4 color)
	{
		if (! m_fontCache.count (font))
		{
			std::cout << "Text::Draw::Font with id'" << font
				<< "' is not loaded" << '\n';
			return;
		}

		m_vao->Bind ();

		std::string::const_iterator c;
		for (c = text.begin (); c != text.end (); c++)
		{
			Glyph g = m_fontCache.at (font).at (*c);

			float xpos = x + g.bearing.x * scale;
			float ypos = y - (g.size.y - g.bearing.y) * scale;

			float w = g.size.x * scale;
			float h = g.size.y * scale;

			std::vector<float> vertices = {
				xpos,		ypos + h,	0.0f, 0.0f,
				xpos,		ypos,		0.0f, 1.0f,
				xpos + w,	ypos + h,	1.0f, 0.0f,
				xpos + w,	ypos,		1.0f, 1.0f
			};

			glBindTexture (GL_TEXTURE_2D, g.id);
			m_vao->m_vbo->setSubData (0,
									  vertices.size (),
									  vertices.data ());
			//glDrawElements (GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, nullptr);
			glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

			x += (g.advance >> 6) * scale;
		}
		m_vao->UnBind ();

		glBindTexture (GL_TEXTURE_2D, 0);
	}
} // end namespace Text