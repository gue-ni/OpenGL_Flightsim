#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Texture_01;

void main()
{             
    vec4 originalColor = texture(u_Texture_01, TexCoords);

#if 0
    int bits_per_color = 5;
    int numColors = 1 << bits_per_color;

    // Scale to range
    vec3 truncatedColor = floor(originalColor.rgb * numColors);  
    
    // Convert back to the 0.0-1.0 range
    vec3 finalColor = truncatedColor / numColors;
    
    // Output the final color
    FragColor = vec4(finalColor, originalColor.a);
#else
    FragColor = originalColor;
#endif

}