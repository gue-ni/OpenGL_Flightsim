#version 330 core

out vec4 FragColor;

uniform float u_Level;
uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;
uniform sampler2D u_Texture;

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
flat in int Factor;

float scale(float input_val, float in_min, float in_max, float out_min, float out_max)
{
    return (input_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


vec3 getNormal(float x, float z)
{
    ivec2 size = textureSize(u_Normalmap, 0);
    ivec2 coord = (size / 2) + ivec2(x, z) / Factor;
    return normalize(texelFetch(u_Normalmap, coord, 0).rgb);
}

vec3 getTexture(float x, float z)
{
    ivec2 size = textureSize(u_Texture, 0);
    ivec2 coord = (size / 2) + ivec2(x, z) / Factor;
    return texelFetch(u_Texture, coord, 0).rgb;
}

vec3 calculateDirLight(vec3 direction, vec3 normal, vec3 color)
{
	float ka = 0.3;
	float kd = 1.0f;

	vec3 light_color = vec3(1.0, 1.0, 1.0);

    // ambient
    vec3 ambient = ka * light_color;
  	
    // diffuse 
    vec3 diffuse = max(dot(normalize(normal), normalize(direction)), 0.0) * light_color;

    return (ambient + diffuse) * color;
}


void main()
{
	if (gl_FrontFacing)
	{
		FragColor = vec4(Color, 1.0); 

		vec3 lightDir = vec3(-2.0, 5.0, -1.0);

		//vec3 normal = getNormal(FragPos.x, FragPos.z);
		vec3 normal = Normal;

		vec3 green = vec3(0.41, 0.55, 0.13);

		vec3 col = getTexture(FragPos.x, FragPos.z);

		FragColor = vec4(calculateDirLight(lightDir, normal, texture(u_Texture, TexCoord).rgb), 1.0);

		//FragColor = vec4(normal, 1.0);
		//FragColor = texture(u_Texture, TexCoord);
	}
	else
	{
		//FragColor = vec4(0.0, 0.0, 1.0, 1.0); // blue
	}
}
