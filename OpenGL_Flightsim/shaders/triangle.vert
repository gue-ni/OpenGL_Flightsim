#version 330 core

layout (location = 0) in vec3 a_Pos;

void main()
{
	gl_Position = vec4(a_Pos.x, a_Pos.y, a_Pos.z, 1.0);
}
