#version 330 core

// Vertex position input
layout (location = 0) in vec3 aPos;

// Uniforms for light-space transformations
uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    // Transform the vertex position to the light's perspective
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
