#version 330 core

out vec4 FragColor;

void main()
{
	if (gl_FrontFacing)
	{
		FragColor = vec4(1.0, 0.0, 0.0, 1.0); // red
	}
	else
	{
		FragColor = vec4(0.0, 0.0, 1.0, 1.0); // blue
	}
}
