#version 330 core
out vec4 FragColor;

//uniform sampler2D u_Texture;

in vec2 TexCoords;
in vec4 Color;
in float FragDepth;

void main()
{
#if 0
	FragColor = texture(u_Texture, TexCoords) * Color;
 #else
	FragColor = Color;
#endif

#if 1
  // log depth buffer
  float far = 150000.0;
  float coeff = 2.0 / (log2(far + 1.0) / 0.693);
  gl_FragDepth = log2(FragDepth) * coeff * 0.5;
#endif

}