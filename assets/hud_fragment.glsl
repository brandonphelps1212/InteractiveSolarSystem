// hud_fragment.glsl  
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform float alpha;

void main()
{    
    // For simple colored quads without texture
    color = vec4(textColor, alpha);
    
    // If we had a font texture, we'd use:
    // vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    // color = vec4(textColor, 1.0) * sampled;
}