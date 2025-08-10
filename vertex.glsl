#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;              // Position of the fragment in world space
out vec3 Normal;               // Normal for lighting
out vec2 TexCoords;            // Texture coordinates
out vec4 FragPosLightSpace;    // Position in light space for shadows

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    // World position of the fragment
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Transform normal to world space
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Pass texture coordinates
    TexCoords = aTexCoords;

    // Calculate position relative to the light's projection
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    // Final clip space position
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
