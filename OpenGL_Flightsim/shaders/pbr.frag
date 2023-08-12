#version 330 core

uniform vec3 u_SolidObjectColor;

uniform sampler2D u_Texture_01;

in vec2 TexCoords;
in vec3 Normal;


vec3 phongLighting(vec3 texColor, vec3 lightDir, vec3 lightColor)
{
  // ambient
  float ka = 0.5;
  vec3 ambient = ka * lightColor;
  
  // diffuse 
  float kd = 0.5f;
  vec3 diffuse = kd * max(dot(Normal, lightDir), 0.0) * lightColor;
  
  // specular
#if 0
  vec3 u_ViewDir = normalize(u_CameraPosition - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);  
  vec3 specular = ks * pow(max(dot(u_ViewDir, reflectDir), 0.0), alpha) * light.color;
#endif

  return (ambient + diffuse) * texColor;
}

void main()
{
#if 0
  gl_FragColor = vec4(u_SolidObjectColor, 1.0);
#else

  vec3 texColor = texture(u_Texture_01, TexCoords).rgb;

  vec3 color = phongLighting(texColor, vec3(0,1,0), vec3(1,1,1));

  gl_FragColor = vec4(color, 1);
 #endif
}