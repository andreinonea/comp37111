#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_texCoords;

uniform mat4 mvp;

out vec2 f_texCoords;

void main()
{
	f_texCoords = a_texCoords;
	gl_Position = mvp * vec4(a_pos, 1.0);
};
