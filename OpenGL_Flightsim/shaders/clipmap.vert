#version 330 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;

void main()
{
    vec3 FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
