#version 330 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;

uniform vec3 u_CameraPos;
uniform sampler2D u_Heightmap;

void main()
{
    vec3 FragPos = vec3(u_Model * vec4(a_Pos, 1.0));

    //vec2 uv = FragPos.xz / textureSize(u_Heightmap, 0);
    //float height = texture(u_Heightmap, uv).x;
    //FragPos.y = height * 5;


    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
