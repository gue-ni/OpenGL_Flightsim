#version 330 core
out vec4 FragColor;

uniform sampler2D u_Texture1;
uniform vec3 u_Albedo;

in vec2 TexCoord;

void main()
{
  //FragColor = texture(u_Texture1, TexCoord);
  FragColor = vec4(u_Albedo,1);
}