#version 330 core

out vec4 FragColor;

uniform float u_Level;

in vec3 Color;

void main()
{
	if (gl_FrontFacing)
	{
		//FragColor = vec4(1.0, u_Level, 0.0, 1.0); // red
		FragColor = vec4(Color, 1.0); 
	}
	else
	{
		FragColor = vec4(0.0, 0.0, 1.0, 1.0); // blue
	}
}
