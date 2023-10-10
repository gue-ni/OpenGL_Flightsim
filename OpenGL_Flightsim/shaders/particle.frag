#version 330 core
out vec4 FragColor;

//uniform sampler2D u_Texture;

in vec2 TexCoords;
in vec4 Color;

void main()
{
	//FragColor = texture(u_Texture, TexCoords) * vec4(u_Color, 1.0f);
	FragColor = Color;
}