#version 330 core
layout (location = 0) in vec3 a_VertexPos;
layout (location = 1) in vec4 a_WorldPosAndSize;
layout (location = 2) in vec4 a_Color;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform vec3 u_Up;
uniform vec3 u_Right;

out vec2 TexCoords;
out vec4 Color;
out float FragDepth;

void main()
{
	vec3 WorldPos = a_WorldPosAndSize.xyz;
	float particleSize = a_WorldPosAndSize.w;

	vec3 vertexPos = WorldPos 
		+ (u_Right * a_VertexPos.x * particleSize) 
		+ (u_Up * a_VertexPos.y * particleSize);

	gl_Position = u_Projection * u_View * vec4(vertexPos, 1.0);

  FragDepth = 1.0 + gl_Position.w;

	TexCoords = a_VertexPos.xy + vec2(0.5, 0.5);
	Color = a_Color;
}