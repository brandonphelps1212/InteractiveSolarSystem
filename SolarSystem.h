#ifndef SOLARSYSTEM_H
#define SOLARSYSTEM_H

#include <vector>
#include <memory>
#include "Sun.h"
#include "Planet.h"
#include "Moon.h"
#include "Shader.h"

class SolarSystem
{
public:
    SolarSystem();
    ~SolarSystem();

    // Initialize the solar system with planets and moons
    void initialize();

    // Update all celestial bodies
    void update(float deltaTime);

    // Render all celestial bodies
    void render(Shader &shader, unsigned int sphereVAO, int vertexCount, const glm::vec3 &cameraPos);

    // Get the sun's position (for lighting)
    glm::vec3 getSunPosition() const;

private:
    std::unique_ptr<Sun> m_sun;
    std::vector<std::shared_ptr<Planet>> m_planets;

    void createPlanets();
    void createMoons();
};

#endif