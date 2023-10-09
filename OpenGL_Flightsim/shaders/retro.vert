#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

out vec3 WorldPos;
noperspective out vec2 TexCoords;
out vec3 Normal;
out vec3 ReflectedVector;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec3 u_CameraPos;

vec4 snap(vec4 vertex, vec2 resolution)
{
    vec4 snappedPos = vertex;
    snappedPos.xyz = vertex.xyz / vertex.w; // convert to normalised device coordinates (NDC)
    snappedPos.xy = floor(resolution * snappedPos.xy) / resolution; // snap the vertex to the lower-resolution grid
    snappedPos.xyz *= vertex.w; // convert back to projection-space
    return snappedPos;
}

void main()
{
  vec4 worldPosition = u_Model * vec4(a_Pos, 1.0f);
  WorldPos = worldPosition.xyz;
  gl_Position = u_Projection * u_View * worldPosition;

  gl_Position = snap(gl_Position, vec2(320, 180));

  TexCoords = a_TexCoord;

  Normal = normalize(mat3(transpose(inverse(u_Model))) * a_Normal);  

  vec3 viewDir = normalize(worldPosition.xyz - u_CameraPos);
  ReflectedVector = reflect(viewDir, Normal);
}
