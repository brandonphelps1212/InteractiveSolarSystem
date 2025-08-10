#ifndef ASTEROIDBELT_H
#define ASTEROIDBELT_H

#include <vector>
#include <glm/glm.hpp>
#include "Shader.h" // REQUIRED for Shader reference

class AsteroidBelt
{
public:
    AsteroidBelt(int numAsteroids, float innerRadius, float outerRadius);
    void render(const Shader &shader);

private:
    std::vector<glm::vec3> positions;
    unsigned int VAO, VBO;
    int asteroidCount;
};

#endif