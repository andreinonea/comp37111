#version 330 core

uniform vec3 u_objectColor;
uniform vec3 u_lightColor;

out vec4 color;

void main()
{
	color = vec4(u_lightColor * u_objectColor, 1.0);
};

