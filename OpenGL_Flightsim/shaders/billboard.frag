#version 330 core
out vec4 FragColor;

uniform vec3			u_Color;
uniform sampler2D u_Texture_01;

in vec2 TexCoords;

void main()
{
	FragColor = texture(u_Texture_01, TexCoords) * vec4(u_Color, 1.0f);
}