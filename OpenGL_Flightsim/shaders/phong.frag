#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;
in vec4 FragPosLightSpace;
  
uniform vec3 camera_Pos; 

uniform vec3 objectColor;

// phong lighting parameters
uniform float ka;
uniform float kd;
uniform float ks;
uniform float alpha;

uniform vec3 backgroundColor;

uniform sampler2D shadowMap;
uniform int useTexture;
uniform sampler2D texture1;


struct Light {
	int type; // POINT = 0, DIRECTIONAL = 1
	vec3 color;
	vec3 position;  
};

#define MAX_LIGHTS 4
uniform int numLights;
uniform int receiveShadow;
uniform Light lights[MAX_LIGHTS];

vec3 getColor()
{
	return useTexture == 1 ? vec3(texture(texture1, TexCoords)) : objectColor;
}

float calculateAttenuation(float constant, float linear, float quadratic, float distance)
{
	return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 u_ProjectionCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    u_ProjectionCoords = u_ProjectionCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, u_ProjectionCoords.xy).r; 

    float currentDepth = u_ProjectionCoords.z;

	float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

vec3 calculateDirLight(Light light)
{
	vec3 direction = light.position;
	
	vec3 color = getColor();

    // ambient
    vec3 ambient = ka * light.color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(direction);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * light.color;
    
    // specular
    vec3 u_ViewDir = normalize(camera_Pos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(u_ViewDir, reflectDir), 0.0), alpha) * light.color;

	float shadow = 0.0;
	if (receiveShadow == 1)
	{
		shadow = calculateShadow(FragPosLightSpace);       
	}

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
}

vec3 calculatePointLight(Light light)
{
	vec3 result;
	vec3 position = light.position;

	vec3 color = getColor();

    // ambient
    vec3 ambient = ka * light.color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(position - FragPos);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * light.color;
    
    // specular
    vec3 u_ViewDir = normalize(camera_Pos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(u_ViewDir, reflectDir), 0.0), alpha) * light.color;

	// attenuation
	float constant		= 1.0;
	float linear		= 0.09;
	float quadratic		= 0.032;
	float distance		= length(position - FragPos);
	float attenuation	= calculateAttenuation(constant, linear, quadratic, distance);  
	
    result += (ambient + diffuse + specular) * attenuation * color;

#if 0
	// https://ijdykeman.github.io/graphics/simple_fog_shader
	vec3 cameraDir = camera_Pos - FragPos;
	vec3 cameraDir = -u_ViewDir;
	float b = length(light.position - camera_Pos);

	float h = length(cross(light.position - camera_Pos, cameraDir)) / length(cameraDir);
	float dropoff = 1.0;
	float fog = (atan(b / h) / (h * dropoff));

	float density = 0.1;

	// TODO: improve this
	float theta = dot(norm, u_ViewDir) >= 0 ? 1 : 0;

	vec3 scattered = light.color * (fog * density) * theta;
	scattered *= calculateAttenuation(constant, linear, quadratic, length(cameraDir));
	
	result += scattered;
#endif

	return result;
}

void main()
{
    //float depthValue = texture(shadowMap, TexCoords).r;
    //FragColor = vec4(vec3(depthValue), 1.0); 
	//return;

	vec3 result;

	for (int i = 0; i < numLights; i++)
	{
		switch (lights[i].type) {
			case 0:
				result += calculatePointLight(lights[i]);
				break;
			
			case 1:
				result += calculateDirLight(lights[i]);
				break;
		}
	}

#if 0	
	// fog

	float tmp = dot(vec3(0,1,0), camera_Pos - FragPos);

	vec4 fogColor = vec4(backgroundColor, 1.0);
	float fogMin = 4.1;
	float fogMax = 100.0;
	float dist = length(camera_Pos - FragPos);
	float fogFactor = (fogMax - dist) / (fogMax - fogMin);

	fogFactor = clamp(fogFactor, 0.0, 1.0);

    FragColor = mix(fogColor * tmp, vec4(result, 1.0), fogFactor);
#else
	FragColor  = vec4(result, 1.0);
#endif
}