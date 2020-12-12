#version 330 core

in vec3 f_texCoords;

uniform samplerCube skybox;

out vec4 color;

void main()
{
	color = texture(skybox, f_texCoords);
};

