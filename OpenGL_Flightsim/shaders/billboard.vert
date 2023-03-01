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

	vec3 vertexPos = u_Position + Right * a_Pos.x * + Up * a_Pos.y;

	gl_Position = u_Projection * u_View * vec4(vertexPos, 1.0);
	gl_Position /= gl_Position.w;
	gl_Position.xy += a_Pos.xy * vec2(0.2, 0.2);
	gl_Position *= vec4(u_Scale, 1.0);

	TexCoords = a_TexCoord;
}