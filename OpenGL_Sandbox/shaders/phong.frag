#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;
in vec4 FragPosLightSpace;
  
uniform vec3 cameraPos; 

uniform vec3 objectColor;

// phong lighting parameters
uniform float ka;
uniform float kd;
uniform float ks;
uniform float alpha;

uniform vec3 backgroundColor;

uniform sampler2D shadowMap;

struct Light {
	int type; // POINT = 0, DIRECTIONAL = 1
	vec3 color;
	vec3 position;  
};

#define MAX_LIGHTS 4
uniform int numLights;
uniform int receiveShadow;
uniform Light lights[MAX_LIGHTS];

float calculateAttenuation(float constant, float linear, float quadratic, float distance)
{
	return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 

    float currentDepth = projCoords.z;

	float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

vec3 calculateDirLight(Light light)
{
	vec3 direction = light.position;
	//vec3 color = vec3(texture(shadowMap, TexCoords.xy));
	vec3 color = objectColor;

    // ambient
    vec3 ambient = ka * light.color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(direction);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * light.color;
    
    // specular
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), alpha) * light.color;

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

    // ambient
    vec3 ambient = ka * light.color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(position - FragPos);
    vec3 diffuse = kd * max(dot(norm, lightDir), 0.0) * light.color;
    
    // specular
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), alpha) * light.color;

	// attenuation
	float constant		= 1.0;
	float linear		= 0.09;
	float quadratic		= 0.032;
	float distance		= length(position - FragPos);
	float attenuation	= calculateAttenuation(constant, linear, quadratic, distance);  
	
    result += (ambient + diffuse + specular) * attenuation * objectColor;

#if 0
	// https://ijdykeman.github.io/graphics/simple_fog_shader
	vec3 cameraDir = cameraPos - FragPos;
	vec3 cameraDir = -viewDir;
	float b = length(light.position - cameraPos);

	float h = length(cross(light.position - cameraPos, cameraDir)) / length(cameraDir);
	float dropoff = 1.0;
	float fog = (atan(b / h) / (h * dropoff));

	float density = 0.1;

	// TODO: improve this
	float theta = dot(norm, viewDir) >= 0 ? 1 : 0;

	vec3 scattered = light.color * (fog * density) * theta;
	scattered *= calculateAttenuation(constant, linear, quadratic, length(cameraDir));
	
	result += scattered;
#endif

	return result;
}

void main()
{
	float depth = texture(shadowMap, TexCoords).x;
    depth = 1.0 - (1.0 - depth) * 25.0;
    FragColor = vec4(depth);
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

	float tmp = dot(vec3(0,1,0), cameraPos - FragPos);

	vec4 fogColor = vec4(backgroundColor, 1.0);
	float fogMin = 4.1;
	float fogMax = 100.0;
	float dist = length(cameraPos - FragPos);
	float fogFactor = (fogMax - dist) / (fogMax - fogMin);

	fogFactor = clamp(fogFactor, 0.0, 1.0);

    FragColor = mix(fogColor * tmp, vec4(result, 1.0), fogFactor);
#else
	FragColor  = vec4(result, 1.0);
#endif
}