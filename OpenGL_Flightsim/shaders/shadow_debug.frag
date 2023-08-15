#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Texture_01;

float near_plane = 1.0;
float far_plane = 20.0;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{             
    float depthValue = texture(u_Texture_01, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}