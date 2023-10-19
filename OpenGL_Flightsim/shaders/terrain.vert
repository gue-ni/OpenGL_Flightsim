#version 330 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;
uniform mat4 u_LightSpaceMatrix;

uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;
uniform sampler2DArray u_TextureArray;

uniform vec3    u_CameraPos;
uniform float   u_Scale;
uniform float   u_SegmentSize;
uniform float   u_Level;
uniform float   u_TerrainSize;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec4 FragPosLightSpace;
out float FragDepth;


flat out float scaleFactor;

vec4 snap(vec4 vertex, vec2 resolution)
{
    vec4 snappedPos = vertex;
    snappedPos.xyz = vertex.xyz / vertex.w; // convert to normalised device coordinates (NDC)
    snappedPos.xy = floor(resolution * snappedPos.xy) / resolution; // snap the vertex to the lower-resolution grid
    snappedPos.xyz *= vertex.w; // convert back to projection-space
    return snappedPos;
}

float scale(float input_val, float in_min, float in_max, float out_min, float out_max)
{
    return (input_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float getHeight(vec2 uv)
{
    vec3 pixel = texture(u_Heightmap, uv).rgb * 255.0;
    return (pixel.r * 256.0 + pixel.g + pixel.b / 256.0) - 32768.0;
}

vec3 getNormal(vec2 uv)
{
    vec3 normal = texture(u_Normalmap, uv).rgb;
    return normalize(normal * 2.0 - 1.0);   
}

vec2 getUV(vec2 pos)
{
    vec2 coord = pos / scaleFactor;
    coord.x = scale(coord.x, -1.0, 1.0, 0.0, 1.0);
    coord.y = scale(coord.y, -1.0, 1.0, 0.0, 1.0);
    return coord;
}


// TODO: figure out in which LOD the current pos is, and then
// sample from the correct level with the correct uv
vec3 sampleTextureClipmap(vec2 pos, vec2 cameraPos)
{
  return vec3(0.0);
}

void main()
{
    // size of the area covered by the heightmap
    scaleFactor = u_TerrainSize / 2.0;

    FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    TexCoords = getUV(FragPos.xz);

    FragPos.y = getHeight(TexCoords);

    Normal = getNormal(TexCoords);

    //Color = vec3(1.0, u_Level, 0.0);
    Color = mix(vec3(1,0,0), vec3(0,0,1), u_Level);

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);

#if 0
    gl_Position = snap(gl_Position, vec2(320, 180));
#endif

  FragDepth = 1.0 + gl_Position.w;
  FragPosLightSpace = u_LightSpaceMatrix * vec4(FragPos, 1.0);

}



