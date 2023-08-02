#version 330 core

out vec4 FragColor;

uniform float u_Level;
uniform vec3 u_Background;
uniform vec3 u_CameraPos;

uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;
uniform sampler2D u_Texture;

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
flat in float Factor;

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

  vec4 fogColor = vec4(u_Background, 1.0);

  float distFromCamera = length(FragPos.xyz - u_CameraPos);
  float fogFactor = (fogMaxdist - distFromCamera) / (fogMaxdist - fogMindist);
  fogFactor = clamp(fogFactor, 0.0, 1.0);

  vec4 terrainColor = vec4(calculateDirLight(lightDir, Normal, texture(u_Texture, TexCoord).rgb), 1.0);
  FragColor = mix(fogColor, terrainColor, fogFactor);
}
