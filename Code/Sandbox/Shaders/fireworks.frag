#version 330 core

uniform vec3 u_lightColor;

out vec4 color;

void main()
{
	color = vec4(u_lightColor, 1.0);
};

