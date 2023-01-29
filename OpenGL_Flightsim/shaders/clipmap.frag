#version 330 core

out vec4 FragColor;

uniform float u_Level;

in vec3 Color;
in vec3 Normal;

vec3 calculateDirLight(vec3 direction)
{
	float ka = 0.3;
	float kd = 1.0f;

	vec3 light_color = vec3(1.0, 1.0, 1.0);

    // ambient
    vec3 ambient = ka * light_color;
  	
    // diffuse 
    vec3 diffuse = max(dot(normalize(Normal), normalize(direction)), 0.0) * light_color;

    return (ambient + diffuse) * Color;
}


void main()
{
	if (gl_FrontFacing)
	{
		FragColor = vec4(Color, 1.0); 

		vec3 tmp = vec3(-2.0, 4.0, -1.0);
		FragColor = vec4(calculateDirLight(tmp), 1.0);

		FragColor = vec4(Normal, 1.0);
	}
	else
	{
		FragColor = vec4(0.0, 0.0, 1.0, 1.0); // blue
	}
}
