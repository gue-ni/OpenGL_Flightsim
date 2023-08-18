#version 330 core

uniform vec3 u_SolidObjectColor;
uniform vec3 u_CameraPos;
uniform vec3 u_LightDir;
uniform vec3 u_LightColor;
uniform sampler2D u_Texture_01;
uniform samplerCube u_EnvMap;
uniform bool u_ShadowPass;

in vec3 Normal;
in vec3 WorldPos;
in vec2 TexCoords;
in vec3 ReflectedVector;

vec3 phongLighting(vec3 texColor, vec3 lightDir, vec3 lightColor) {
  // TODO: remove hardcoded values
  float ka = 0.5;
  float kd = 0.5;
  float ks = 0.1;
  float alpha = 20.0;

  // ambient
  vec3 ambient = ka * lightColor;
  
  // diffuse 
  vec3 diffuse = kd * max(dot(Normal, lightDir), 0.0) * lightColor;
  
  // specular
  vec3 viewDir = normalize(u_CameraPos - WorldPos);
  vec3 reflectDir = reflect(-lightDir, Normal);  
  vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), alpha) * lightColor;

  return (ambient + diffuse + specular) * texColor;
}

void main() {
  if (u_ShadowPass) {
    // only write to depth buffer
    return;
  }

  vec3 texColor = texture(u_Texture_01, TexCoords).rgb;
  vec3 reflectedColor = texture(u_EnvMap, ReflectedVector).rgb;

  float shininess = 0.3;

  vec3 color = phongLighting(mix(texColor, reflectedColor, shininess), u_LightDir, u_LightColor);

  gl_FragColor = vec4(color, 1.0);
}