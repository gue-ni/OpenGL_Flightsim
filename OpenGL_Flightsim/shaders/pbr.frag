#version 330 core
out vec4 FragColor;

uniform sampler2D u_Texture1;

in vec2 TexCoord;

void main()
{
  FragColor = vec4(texture(u_Texture1, TexCoord), 1.0f);
}