#version 330 core
//out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_Texture_01;

void main()
{    
    gl_FragColor = texture(u_Texture_01, TexCoords);
}