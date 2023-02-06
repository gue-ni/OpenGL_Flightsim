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

vec3 calculateDirLight(vec3 direction, vec3 normal, vec3 color)
{
	float ka = 0.6;
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


		vec3 lightDir = vec3(-2.0, 4.0, -1.0);
		FragColor = vec4(calculateDirLight(lightDir, Normal, texture(u_Texture, TexCoord).rgb), 1.0);
		//FragColor = vec4(Normal, 1.0);
		//FragColor = texture(u_Texture, TexCoord);
		//FragColor = vec4(Color, 1.0); 
	}
	else
	{
		//FragColor = vec4(0.0, 0.0, 1.0, 1.0); // blue
	}
}
