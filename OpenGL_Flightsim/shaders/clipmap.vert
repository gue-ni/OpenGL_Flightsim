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

out int Factor;

float scale(float input_val, float in_min, float in_max, float out_min, float out_max)
{
    return (input_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



float getHeight(float x, float z)
{
    ivec2 size = textureSize(u_Heightmap, 0);

    ivec2 coord = (size / 2) + ivec2(x, z) / Factor;

    float sample = texelFetch(u_Heightmap, coord, 0).r;

    float scale = 7000;
    float shift = -2000;
    return scale * sample + shift;
}

float getHeight2(float x, float z)
{
    vec2 coord = vec2(x,z) / Factor;
    coord.x = scale(coord.x, -1.0, 1.0, 0.0, 1.0);
    coord.y = scale(coord.y, -1.0, 1.0, 0.0, 1.0);

    float sample = texture(u_Heightmap, coord).r;
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

vec3 getNormal2(float x, float z)
{

    vec2 coord = vec2(x,z) / Factor;
    coord.x = scale(coord.x, -1.0, 1.0, 0.0, 1.0);
    coord.y = scale(coord.y, -1.0, 1.0, 0.0, 1.0);


    return normalize(texture(u_Normalmap, coord).rgb);
}


void main()
{
    Factor = 15000;
    FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    FragPos.y = getHeight2(FragPos.x, FragPos.z);
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);

	//apply nostalgic vertex jitter
    //float positionResolution = 128;
    //float distanceFromCam = clamp(gl_Position.w, -0.1, 1000);
	//gl_Position.xy = round(gl_Position.xy * (positionResolution / distanceFromCam)) / (positionResolution / distanceFromCam);

    Normal = getNormal2(FragPos.x, FragPos.z);
	Color = vec3(1.0, u_Level, 0.0);
}
