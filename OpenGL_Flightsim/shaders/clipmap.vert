#version 330 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;

uniform vec3 u_CameraPos;
uniform sampler2D u_Heightmap;

out vec3 Color;

void main()
{
    vec3 FragPos = vec3(u_Model * vec4(a_Pos, 1.0));



    ivec2 size = textureSize(u_Heightmap, 0);

    int factor = 25;
    ivec2 coord = (size / 2) + ivec2(FragPos.x, FragPos.z) / factor;

    //ivec2 coord = ivec2(FragPos.x, FragPos.z) / factor;

    
    coord = abs(coord);

    Color = texelFetch(u_Heightmap, coord, 0).rgb;



    float scale = 10000;
    float shift = -5000;
    float y = scale * Color.r + shift;
    FragPos.y = y;


    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
