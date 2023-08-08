#version 330 core
out vec4 FragColor;

uniform vec3 u_Albedo;

void main()
{
  FragColor = vec4(u_Albedo, 1.0f);
}