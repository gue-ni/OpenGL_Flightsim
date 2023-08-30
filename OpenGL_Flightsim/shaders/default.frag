#version 330 core

uniform vec3 u_SolidObjectColor;
uniform vec3 u_CameraPos;
uniform vec3 u_LightDir;
uniform vec3 u_LightColor;

uniform samplerCube u_EnvMap;
uniform sampler2D u_ShadowMap;
uniform sampler2D u_Texture_01;

uniform bool u_ShadowPass;
uniform bool u_ReceiveShadow;

uniform float u_Shininess;

in vec3 Normal;
in vec3 WorldPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in float FragDepth;



void main() {

#if 0
  // Debug, Normal as color
  gl_FragColor = vec4((normalize(Normal) * 0.5) + 0.5, 1.0);
  return;
#endif

#if 1
  if (u_ShadowPass) {
    // only write to depth buffer
    gl_FragDepth = gl_FragCoord.z;
    return;
  }

  float farPlane = 150000.0;
  float coeff = 2.0 / (log2(farPlane + 1.0) / 0.693);
  gl_FragDepth = log2(FragDepth) * coeff * 0.5;
#endif

  gl_FragColor = texture(u_Texture_01, TexCoords);
}