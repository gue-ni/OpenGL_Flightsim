#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;
in vec4 FragPosLightSpace;
  
uniform vec3 u_CameraPosition; 

// phong lighting parameters
uniform float ka;
uniform float kd;
uniform float ks;
uniform float alpha;

uniform bool u_ReceiveShadow;
uniform sampler2D u_ShadowMap;

uniform bool u_UseTexture;
uniform sampler2D u_Texture1;

uniform vec3 u_SolidObjectColor;
uniform vec3 u_BackgroundColor;

struct Light {
	int type; // POINT = 0, DIRECTIONAL = 1
	vec3 color;
	vec3 position;  
};

#define MAX_LIGHTS 4
uniform int u_NumLights;
uniform Light u_Lights[MAX_LIGHTS];

vec3 getColor()
{
	return u_UseTexture ? vec3(texture(u_Texture1, TexCoords)) : u_SolidObjectColor;
	//return vec3(1, 0, 0);

}

float calculateAttenuation(float constant, float linear, float quadratic, float distance)
{
	return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 projectionCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    float closestDepth = texture(u_ShadowMap, projectionCoords.xy).r; 
    float currentDepth = projectionCoords.z;
	float bias = 0.005;
    return (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
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
    vec3 u_ViewDir = normalize(u_CameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    vec3 specular = ks * pow(max(dot(u_ViewDir, reflectDir), 0.0), alpha) * light.color;

	float shadow = 0.0;
	if (u_ReceiveShadow)
	{
		shadow = calculateShadow(FragPosLightSpace);       
	}

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
	//return color;
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
    vec3 u_ViewDir = normalize(u_CameraPosition - FragPos);
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
	vec3 cameraDir = u_CameraPosition - FragPos;
	vec3 cameraDir = -u_ViewDir;
	float b = length(light.position - u_CameraPosition);

	float h = length(cross(light.position - u_CameraPosition, cameraDir)) / length(cameraDir);
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
    //float depthValue = texture(u_ShadowMap, TexCoords).r;
    //FragColor = vec4(vec3(depthValue), 1.0); 
	//return;

	vec3 result;

	for (int i = 0; i < u_NumLights; i++)
	{
		switch (u_Lights[i].type) {
			case 0:
				result += calculatePointLight(u_Lights[i]);
				break;
			
			case 1:
				result += calculateDirLight(u_Lights[i]);
				break;
		}
	}

#if 0	
	// fog

	float tmp = dot(vec3(0,1,0), u_CameraPosition - FragPos);

	vec4 fogColor = vec4(u_BackgroundColor, 1.0);
	float fogMin = 4.1;
	float fogMax = 100.0;
	float dist = length(u_CameraPosition - FragPos);
	float fogFactor = (fogMax - dist) / (fogMax - fogMin);

	fogFactor = clamp(fogFactor, 0.0, 1.0);

    FragColor = mix(fogColor * tmp, vec4(result, 1.0), fogFactor);
#else
	FragColor  = vec4(result, 1.0);
#endif
}