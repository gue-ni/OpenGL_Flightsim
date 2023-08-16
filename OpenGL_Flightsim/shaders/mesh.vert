#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

out vec3 Normal;
out vec3 WorldPos;
out vec2 TexCoords;
out vec3 ReflectedVector;

uniform mat4 u_View;
uniform mat4 u_Model;
uniform vec3 u_CameraPos;
uniform mat4 u_Projection;
uniform mat4 u_LightSpaceMatrix;
uniform bool u_ShadowPass;

void main() {
  if (u_ShadowPass) {
    // render from pov of light source
    gl_Position = u_LightSpaceMatrix * u_Model * vec4(a_Pos, 1.0);
    return;
  }

  vec4 worldPosition = u_Model * vec4(a_Pos, 1.0f);
  WorldPos = worldPosition.xyz;

  gl_Position = u_Projection * u_View * worldPosition;

  TexCoords = a_TexCoord;

  Normal = normalize(mat3(transpose(inverse(u_Model))) * a_Normal);

  vec3 viewDir = normalize(worldPosition.xyz - u_CameraPos);

  ReflectedVector = reflect(viewDir, Normal);
}
