#version 330 core
out vec4 FragColor;

uniform bool u_UseTexture;
uniform sampler2D u_Texture;
uniform vec3 u_SolidObjectColor;

in vec2 TexCoords;

void main()
{
	//FragColor = vec4(u_SolidObjectColor.rgb, 1.0f);
	//FragColor = vec4(vec3(1,0,0), 1.0f);

	FragColor = u_UseTexture ? texture(u_Texture, TexCoords) : vec4(u_SolidObjectColor, 1.0);
}