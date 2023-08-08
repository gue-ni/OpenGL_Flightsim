#version 330 core
out vec4 FragColor;

uniform vec3 u_SolidObjectColor;

uniform sampler2D u_Texture1;

in vec2 TexCoords;

void main()
{
#if 0
  FragColor = vec4(u_SolidObjectColor, 1.0);
#else
  FragColor = texture(u_Texture1, TexCoords);
 #endif
}