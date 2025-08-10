#version 330 core
in vec4 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vColor; // premult not required; we use straight alpha blending
}
