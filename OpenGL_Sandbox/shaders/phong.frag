#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{

    float ka = 0.5;
    float kd = 0.5;
    float ks = 0.0;

    // ambient
    vec3 ambient = ka * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * lightColor;
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = ks * spec * lightColor;  
        
    vec3 result = (ambient * diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
} 