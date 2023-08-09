#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Texture_01;

void main()
{             
    //float depthValue = texture(u_ShadowMap, TexCoords).r;
    //FragColor = vec4(vec3(depthValue), 1.0); 
    FragColor = texture(u_Texture_01, TexCoords);
}