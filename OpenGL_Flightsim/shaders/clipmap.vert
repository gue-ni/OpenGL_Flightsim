#version 330 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;

uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;

uniform vec3    u_CameraPos;
uniform float   u_Scale;
uniform float   u_SegmentSize;
uniform float   u_Level;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

flat out int Factor;

float scale(float input_val, float in_min, float in_max, float out_min, float out_max)
{
    return (input_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float getHeight(vec2 uv)
{
    if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.x > 1)
    {
        return 0.0;
    }

    float height = texture(u_Heightmap, uv).r;

    float scale = 3000;
    float shift = 0;
    return scale * height + shift;
}

vec3 getNormal(vec2 uv)
{
    return normalize(texture(u_Normalmap, uv).rgb);
}

vec2 getUV(vec2 pos)
{
    vec2 coord = pos / Factor;
    coord.x = scale(coord.x, -1.0, 1.0, 0.0, 1.0);
    coord.y = scale(coord.y, -1.0, 1.0, 0.0, 1.0);
    return coord;
}

void main()
{
    Factor = 25000;

    FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    TexCoord = getUV(FragPos.xz);

    FragPos.y = getHeight(TexCoord);

    Normal = getNormal(TexCoord);

	  Color = vec3(1.0, u_Level, 0.0);

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}



