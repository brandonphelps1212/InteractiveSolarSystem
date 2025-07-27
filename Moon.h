#ifndef MOON_H
#define MOON_H

#include "CelestialBody.h"

// Forward declaration
class Planet;

class Moon : public CelestialBody
{
public:
    Moon(const std::string &name, float radius, const std::string &texturePath,
         Planet *parentPlanet, float orbitRadius, float orbitSpeed);

    void update(float deltaTime) override;
    void render(Shader &shader, unsigned int sphereVAO, int vertexCount) override;
    glm::mat4 getModelMatrix() const override;

private:
    Planet *m_parentPlanet;
    float m_orbitRadius;
    float m_orbitSpeed;
    float m_orbitAngle;
    float m_time;

    void updateOrbitalPosition();
};

#endif