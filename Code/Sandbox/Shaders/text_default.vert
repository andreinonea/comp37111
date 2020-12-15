#version 330 core

layout(location = 0) in vec4 vertex;

uniform mat4 projection;

out vec2 f_texCoords;

void main()
{
	f_texCoords = vertex.zw;
	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
};
