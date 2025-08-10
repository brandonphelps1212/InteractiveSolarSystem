#version 330 core

// This shader doesn't output color, only depth
void main()
{
    // The default depth value (from gl_Position.z) is written automatically
    // No color output required for shadow map
}