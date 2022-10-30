#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 camera; 

uniform vec3 lightPos; 
uniform vec3 lightColor;

uniform vec3 objectColor;

// phong parameters
uniform float ka;
uniform float kd;
uniform float ks;
uniform float alpha;

void main()
{

    // ambient
    vec3 ambient = ka * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * lightColor;
    
    // specular
    vec3 viewDir = normalize(camera - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), alpha) * lightColor;
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
} 