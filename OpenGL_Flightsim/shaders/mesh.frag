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
in vec3 ReflectedVector;
in vec4 FragPosLightSpace;
in float FragDepth;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_ShadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective


    float bias = max(0.05 * (1.0 - dot(Normal, u_LightDir)), 0.001);  
    bias = 0.00001;


    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    if(projCoords.z > 1.0)
    {
        shadow = 0.0;
    }

    return shadow;
}

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


  if (u_ReceiveShadow) {
    float shadow = ShadowCalculation(FragPosLightSpace);                      
    return (ambient + (1.0 - shadow) * (diffuse + specular)) * texColor;
  } else {
    return (ambient + diffuse + specular) * texColor;
  }
}

void main() {

#if 0
  // Debug, Normal as color
  gl_FragColor = vec4((normalize(Normal) * 0.5) + 0.5, 1.0);
  return;
#endif

  if (u_ShadowPass) {
    // only write to depth buffer
    gl_FragDepth = gl_FragCoord.z;
    return;
  }

  float farPlane = 150000.0;
  float coeff = 2.0 / (log2(farPlane + 1.0) / 0.693);
  gl_FragDepth = log2(FragDepth) * coeff * 0.5;

  vec3 texColor = texture(u_Texture_01, TexCoords).rgb;
  vec3 reflectedColor = texture(u_EnvMap, ReflectedVector).rgb;

  vec3 color = phongLighting(mix(texColor, reflectedColor, u_Shininess), u_LightDir, u_LightColor);

  gl_FragColor = vec4(color, 1.0);
}