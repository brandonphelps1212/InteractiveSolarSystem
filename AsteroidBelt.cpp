#include "AsteroidBelt.h"
#include <GL/glew.h>
#include <cstdlib>
#include <cmath>
#include "Shader.h"

AsteroidBelt::AsteroidBelt(int numAsteroids, float innerRadius, float outerRadius)
    : asteroidCount(numAsteroids)
{
    // Generate random positions for asteroids in a ring
    for (int i = 0; i < asteroidCount; ++i)
    {
        float angle = static_cast<float>(rand()) / RAND_MAX * 360.0f;
        float distance = innerRadius + static_cast<float>(rand()) / RAND_MAX * (outerRadius - innerRadius);

        float x = cos(glm::radians(angle)) * distance;
        float z = sin(glm::radians(angle)) * distance;
        float y = ((rand() % 100) / 100.0f - 0.5f) * 0.1f; // Small vertical variation

        positions.emplace_back(glm::vec3(x, y, z));
    }

    // Setup VAO and VBO for points
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void AsteroidBelt::render(const Shader &shader)
{
    shader.use(); // Use the asteroid particle shader
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, asteroidCount);
    glBindVertexArray(0);
}