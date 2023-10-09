#version 330 core

out vec4 FragColor;

uniform float u_Level;
uniform vec3 u_FogColor;
uniform vec3 u_CameraPos;

uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;
uniform sampler2D u_Texture_01;


uniform sampler2D u_Shadowmap;

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in float FragDepth;


flat in float scaleFactor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_Shadowmap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    if(projCoords.z > 1.0)
    {
        shadow = 0.0;
    }

    return shadow;
}

vec3 phongLighting(vec3 texColor, vec3 lightDir, vec3 lightColor)
{
  // TODO: remove hardcoded values
  float ka = 0.5;
  float kd = 0.5;
  float ks = 0.1;
  float alpha = 20.0;

  // ambient
  vec3 ambient = ka * lightColor;
  
  // diffuse 
  vec3 diffuse = kd * max(dot(Normal, lightDir), 0.0) * lightColor;

  diffuse = vec3(0.5, 0.5, 0.5);

  
  // specular
  vec3 viewDir = normalize(u_CameraPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, Normal);  
  vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), alpha) * lightColor;

 // float shadow = 0.0;
 float shadow = ShadowCalculation(FragPosLightSpace);                      

  return (ambient + (1.0 - shadow) * (diffuse + specular)) * texColor;
}

void main()
{
  vec3 lightDir = vec3(-2.0, 4.0, -1.0);

  // Calculate fog
  float fogMindist = 1000.0;
  float fogMaxdist = 75000.0;

  vec4 fogColor = vec4(u_FogColor, 1.0);
  float distFromCamera = length(FragPos.xyz - u_CameraPos);
  float fogscaleFactor = (fogMaxdist - distFromCamera) / (fogMaxdist - fogMindist);
  fogscaleFactor = clamp(fogscaleFactor, 0.0, 1.0);

  //vec4 terrainColor = vec4(calculateDirLight(lightDir, Normal, texture(u_Texture_01, TexCoord).rgb), 1.0);


  vec3 LightDir = vec3(0,1,0);
  vec3 LightColor = vec3(1.0);
  
  vec3 texColor = texture(u_Texture_01, TexCoords).rgb;
  vec4 terrainColor = vec4(phongLighting(texColor, LightDir, LightColor), 1.0);

#if 1
  FragColor = mix(fogColor, terrainColor, fogscaleFactor);
#else
  FragColor = mix(vec4(Color, 1.0), terrainColor, 0.5);
#endif

  float farPlane = 150000.0;
  float coeff = 2.0 / (log2(farPlane + 1.0) / 0.693);
  gl_FragDepth = log2(FragDepth) * coeff * 0.5;

  //FragColor = terrainColor;
}
