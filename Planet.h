#ifndef PLANET_H
#define PLANET_H

#include "CelestialBody.h"
#include <vector>
#include <memory>

// Forward declaration
class Moon;

class Planet : public CelestialBody
{
public:
    Planet(const std::string &name, float radius, const std::string &texturePath,
           float orbitRadius, float orbitSpeed, float rotationSpeed);

    void update(float deltaTime) override;
    void render(Shader &shader, unsigned int sphereVAO, int vertexCount) override;
    glm::mat4 getModelMatrix() const override;

    // Moon management
    void addMoon(std::shared_ptr<Moon> moon);
    std::vector<std::shared_ptr<Moon>> &getMoons() { return m_moons; }

    // Orbital mechanics
    float getOrbitRadius() const { return m_orbitRadius; }
    float getOrbitAngle() const { return m_orbitAngle; }

private:
    float m_orbitRadius;
    float m_orbitSpeed;
    float m_orbitAngle;
    float m_time;

    std::vector<std::shared_ptr<Moon>> m_moons;

    void updateOrbitalPosition();
};

#endif