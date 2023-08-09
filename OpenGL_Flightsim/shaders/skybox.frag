#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_Texture1;

void main()
{    
    FragColor = texture(u_Texture1, TexCoords);
}