#version 330 core
layout (location = 0) in vec3 a_VertexPos;
layout (location = 1) in vec3 a_WorldPos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform vec3 u_Up;
uniform vec3 u_Right;

out vec2 TexCoords;

void main()
{
	float particleSize = 1.0;

	vec3 vertexPos = a_WorldPos 
		+ (u_Right * a_VertexPos.x * particleSize) 
		+ (u_Up * a_VertexPos.y * particleSize);

	gl_Position = u_Projection * u_View * vec4(vertexPos, 1.0);

	TexCoords = a_VertexPos.xy + vec2(0.5, 0.5);
}