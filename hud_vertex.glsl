#version 330 core
layout (location = 0) in vec3 aPos;     // pixel-space
layout (location = 1) in vec4 aColor;   // normalized from ubyte

uniform mat4 uProjection;

out vec4 vColor;

void main()
{
    vColor = aColor;
    gl_Position = uProjection * vec4(aPos, 1.0);
}
