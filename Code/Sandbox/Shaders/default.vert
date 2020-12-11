#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec2 vTexCoords;

uniform mat4 mvp;

out vec2 fTexCoords;

void main()
{
	gl_Position = mvp * vec4(vPos, 1.0);
	fTexCoords = vTexCoords;
};
