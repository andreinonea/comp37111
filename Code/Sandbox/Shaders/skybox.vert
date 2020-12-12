#version 330 core

layout(location = 0) in vec3 a_pos;

uniform mat4 mvp;

out vec3 f_texCoords;

void main()
{
	f_texCoords = a_pos;
	vec4 pos = mvp * vec4(a_pos, 1.0);
	gl_Position = pos.xyww;
};
