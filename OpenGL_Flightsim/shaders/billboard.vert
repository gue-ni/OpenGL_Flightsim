#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_Position;
uniform vec3 u_Up;
uniform vec3 u_Right;
uniform vec3 u_Scale;

out vec2 TexCoords;

void main()
{
	vec3 Up = u_Up;
	vec3 Right = u_Right;
	vec3 Pos = a_Pos * u_Scale;
	//vec3 Pos = a_Pos;

	vec3 vertexPos = u_Position + Right * Pos.x * + Up * Pos.y;

	gl_Position = u_Projection * u_View * vec4(vertexPos, 1.0);
	gl_Position /= gl_Position.w;
	gl_Position.xy += Pos.xy * vec2(0.5, 0.5);

	TexCoords = a_TexCoord;
}