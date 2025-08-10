#ifndef PLANET_H
#define PLANET_H

#include "CelestialBody.h"
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>

class Moon;

class Planet : public CelestialBody
{
public:
    // Now accepts multiple facts
    Planet(const std::string &name, float radius, const std::string &texturePath,
           float orbitRadius, float orbitSpeed, float rotationSpeed,
           const std::vector<std::string> &facts = {});

    void update(float deltaTime) override;
    void render(Shader &shader, unsigned int sphereVAO, int vertexCount) override;
    glm::mat4 getModelMatrix() const override;

    // Moon management
    void addMoon(std::shared_ptr<Moon> moon);
    std::vector<std::shared_ptr<Moon>> &getMoons() { return m_moons; }

    // Orbital mechanics
    float getOrbitRadius() const { return m_orbitRadius; }
    float getOrbitAngle() const { return m_orbitAngle; }

    // Selection highlight
    void setSelected(bool s) { m_selected = s; }

    // Facts
    // Choose a new random fact (avoids repeating the last one when possible)
    std::string chooseRandomFact();
    // Returns the currently chosen fact (or the first if none chosen yet)
    const std::string &currentFact() const;

    // Picking
    bool intersectsRay(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, float &tOut) const;

private:
    float m_orbitRadius;
    float m_orbitSpeed;
    float m_orbitAngle;
    float m_time;

    bool m_selected = false;
    std::vector<std::shared_ptr<Moon>> m_moons;

    // Multiple facts
    std::vector<std::string> m_facts;
    int m_lastFactIndex = -1;

    void updateOrbitalPosition();
};

#endif
