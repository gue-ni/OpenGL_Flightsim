#version 330 core
layout (location = 0) in vec3 a_Pos;

out vec3 TexCoords;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    // remove translation from view matrix
    mat4 view = mat4
    (
      vec4(u_View[0].xyz,0),
      vec4(u_View[1].xyz,0),
      vec4(u_View[2].xyz,0),
      vec4(0,0,0,1)
    );

    TexCoords = a_Pos;
#if 1
    vec4 pos = u_Projection * view * u_Model * vec4(a_Pos, 1.0);
    gl_Position = pos;
#else
    vec4 pos = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);
    gl_Position = pos.xyww;
#endif
}  