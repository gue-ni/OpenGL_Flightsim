#version 330 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;

uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;

uniform vec3 u_CameraPos;
uniform float u_Scale;
uniform float u_SegmentSize;
uniform float u_Level;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;

int Factor = 1;

float getHeight(float x, float z)
{
    ivec2 size = textureSize(u_Heightmap, 0);

    ivec2 coord = (size / 2) + ivec2(x, z) / Factor;

    float sample = texelFetch(u_Heightmap, coord, 0).r;

    float scale = 7000;
    float shift = -2000;
    return scale * sample + shift;
}

vec3 getNormal(float x, float z)
{
    ivec2 size = textureSize(u_Normalmap, 0);
    ivec2 coord = (size / 2) + ivec2(x, z) / Factor;
    return normalize(texelFetch(u_Normalmap, coord, 0).rgb);
}

void main()
{
    Factor = 25;
    FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    FragPos.y = getHeight(FragPos.x, FragPos.z);

    Normal = getNormal(FragPos.x, FragPos.z);

	Color = vec3(1.0, u_Level, 0.0);

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
