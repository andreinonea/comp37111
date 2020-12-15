#version 330 core

uniform vec4 u_lightColor;

out vec4 color;

void main()
{
	color = u_lightColor;
};

