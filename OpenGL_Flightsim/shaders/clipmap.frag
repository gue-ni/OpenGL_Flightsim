#version 330 core

out vec4 FragColor;

uniform float u_Level;
uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;


int factor = 25;

vec3 getNormal(float x, float z)
{
    ivec2 size = textureSize(u_Normalmap, 0);
    ivec2 coord = (size / 2) + ivec2(x, z) / factor;
    return normalize(texelFetch(u_Normalmap, coord, 0).rgb);
}

vec3 calculateDirLight(vec3 direction, vec3 normal)
{
	float ka = 0.3;
	float kd = 1.0f;

	vec3 light_color = vec3(1.0, 1.0, 1.0);

    // ambient
    vec3 ambient = ka * light_color;
  	
    // diffuse 
    vec3 diffuse = max(dot(normalize(normal), normalize(direction)), 0.0) * light_color;

    return (ambient + diffuse) * Color;
}


void main()
{
	if (gl_FrontFacing)
	{
		FragColor = vec4(Color, 1.0); 

		vec3 lightDir = vec3(-2.0, 4.0, -1.0);

		//vec3 normal = getNormal(FragPos.x, FragPos.z);
		vec3 normal = Normal;

		FragColor = vec4(calculateDirLight(lightDir, normal), 1.0);

		//FragColor = vec4(normal, 1.0);
	}
	else
	{
		FragColor = vec4(0.0, 0.0, 1.0, 1.0); // blue
	}
}
