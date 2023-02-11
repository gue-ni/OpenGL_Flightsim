#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_LightSpaceMatrix;

void main()
{

    FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);

	//apply nostalgic vertex jitter
    //float distanceFromCam = clamp(gl_Position.w, -0.1, 1000);
    //float positionResolution = 128;
	//gl_Position.xy = round(gl_Position.xy * (positionResolution / distanceFromCam)) / (positionResolution / distanceFromCam);


    TexCoords = a_TexCoord;
    Normal = mat3(transpose(inverse(u_Model))) * a_Normal;  
    FragPosLightSpace = u_LightSpaceMatrix * vec4(FragPos, 1.0);
}