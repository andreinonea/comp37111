#version 330 core

in vec2 f_texCoords;

uniform sampler2D text;
uniform vec4 textColor;

out vec4 color;

void main()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, f_texCoords).r);
	color = textColor * sampled;
};

