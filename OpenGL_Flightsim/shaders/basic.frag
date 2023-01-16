#version 330 core
out vec4 FragColor;

uniform vec3 u_SolidObjectColor;

void main()
{
	FragColor = vec4(u_SolidObjectColor.rgb, 1.0f);
}