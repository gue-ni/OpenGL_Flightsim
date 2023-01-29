#version 330 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;

uniform vec3 u_CameraPos;
uniform sampler2D u_Heightmap;
uniform float u_Scale;
uniform float u_SegmentSize;
uniform float u_Level;

out vec3 Color;
out vec3 Normal;

float getHeight(float x, float z)
{
    ivec2 size = textureSize(u_Heightmap, 0);

    int factor = 25;
    ivec2 coord = (size / 2) + ivec2(x, z) / factor;

    float sample = texelFetch(u_Heightmap, coord, 0).r;

    float scale = 10000;
    float shift = -2000;
    return scale * sample + shift;
}

void main()
{
    vec3 FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    FragPos.y = getHeight(FragPos.x, FragPos.z);

    float f = 1.0;
    vec3 a = vec3(FragPos.x + u_SegmentSize * f, getHeight(FragPos.x + u_SegmentSize * f, FragPos.z), FragPos.z);
    vec3 b = vec3(FragPos.x, getHeight(FragPos.x, FragPos.z + u_SegmentSize * f), FragPos.z + u_SegmentSize * f);

    vec3 v0 = a - FragPos;
	vec3 v1 = b - FragPos;
	Normal = normalize(cross(v1, v0));

	Color = vec3(1.0, u_Level, 0.0);

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
