#version 330 core

out vec4 FragColor;

uniform float u_Level;
uniform vec3 u_FogColor;
uniform vec3 u_CameraPos;

uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;
uniform sampler2D u_Texture_01;

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
flat in float scaleFactor;
//noperspective in vec2 TexCoord;
in vec2 TexCoord;

vec3 calculateDirLight(vec3 lightDirection, vec3 normal, vec3 color)
{
  float ka = 0.6;
  float kd = 1.0f;

  vec3 lightColor = vec3(1.0);

  // ambient
  vec3 ambient = ka * lightColor;
  
  // diffuse 
  vec3 diffuse = max(dot(normalize(normal), normalize(lightDirection)), 0.0) * lightColor;

  return (ambient + diffuse) * color;
}

void main()
{
  vec3 lightDir = vec3(-2.0, 4.0, -1.0);

  // Calculate fog
  float fogMindist = 1000.0;
  float fogMaxdist = 100000.0;

  vec4 fogColor = vec4(u_FogColor, 1.0);

  float distFromCamera = length(FragPos.xyz - u_CameraPos);
  float fogscaleFactor = (fogMaxdist - distFromCamera) / (fogMaxdist - fogMindist);
  fogscaleFactor = clamp(fogscaleFactor, 0.0, 1.0);

  vec4 terrainColor = vec4(calculateDirLight(lightDir, Normal, texture(u_Texture_01, TexCoord).rgb), 1.0);

#if 1
  FragColor = mix(fogColor, terrainColor, fogscaleFactor);
#else
  FragColor = mix(vec4(Color, 1.0), terrainColor, 0.5);
#endif
}
