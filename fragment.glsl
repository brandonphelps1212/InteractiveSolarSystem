#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;  // Planet texture
uniform sampler2D shadowMap;         // Shadow depth map
uniform vec3 lightPos;               // Sun position
uniform vec3 viewPos;                // Camera position

// Atmosphere
uniform vec3 atmosphereColor;        // Glow color
uniform float atmosphereIntensity;   // Glow strength

// Shadow calculation
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // Transform to normalized device coordinates
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // If outside the light's range
    if (projCoords.z > 1.0)
        return 0.0;

    // Retrieve closest depth value from shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r;  
    float currentDepth = projCoords.z;

    // Bias to reduce shadow acne
    float bias = max(0.005 * (1.0 - dot(Normal, normalize(lightPos - FragPos))), 0.0005);

    // Apply shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

void main()
{
    // Base texture color
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 normal = normalize(Normal);

    // Lighting calculations
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color;

    // Ambient light
    vec3 ambient = 0.2 * color;

    // Calculate shadow factor
    float shadow = ShadowCalculation(FragPosLightSpace);

    // Combine ambient and diffuse lighting
    vec3 lighting = (ambient + (1.0 - shadow) * diffuse);

    // Calculate atmosphere glow using Fresnel effect
    vec3 viewDir = normalize(viewPos - FragPos);
    float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), 3.0);
    vec3 atmosphere = atmosphereColor * fresnel * atmosphereIntensity;

    // Final color
    vec3 finalColor = lighting + atmosphere;
    FragColor = vec4(finalColor, 1.0);
}